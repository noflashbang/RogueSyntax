#include "UI_Layout_TextArea.h"
#include <iostream>
#include <array>

std::vector<std::string> GetLinesBySplitor(const std::string& text, char splitor)
{
	std::vector<std::string> lines;
	size_t start = 0;
	size_t end = text.find(splitor);
	while (end != std::string::npos)
	{
		lines.push_back(text.substr(start, end - start));
		start = end + 1;
		end = text.find(splitor, start);
	}
	lines.push_back(text.substr(start, end - start));
	return lines;
}

UI_TextArea::UI_TextArea(const UIConfig& config, const std::string name, std::shared_ptr<UIEventObserver<std::string>> focusChanged, std::unique_ptr<ILineNumberingStrategy> numbering)
	: _config(config), _eventCurrentFocusObserver(focusChanged), _name(name), _lineNumberingStrategy(std::move(numbering))
{
	_cursorPosition = { 0, 0 };
	_highlightPosition = { 0, 0 };
	_hoverPosition = { 0, 0 };
	_highlighting = false;

	_scrollOffset = { 0, 0 };

	_textboxes.push_back(std::make_unique<UI_Textbox>(_config, TextboxName(), _eventTextboxFocus.Subscribe(), std::make_unique<BarCursorStrategy>(_config)));
	_scrollBarH = std::make_unique<UI_ScrollBar>(_config, 0, 0, 0, 1, SCROLLBAR_HORIZONTAL);
	_scrollBarV = std::make_unique<UI_ScrollBar>(_config, 0, 0, 0, 1, SCROLLBAR_VERTICAL);
}

void UI_TextArea::Layout()
{
	if (!_highlighting)
	{
		for (auto& textbox : _textboxes)
		{
			textbox->SetHighlighting(false);
		}
	}

	if (_allowScrolling)
	{
		LayoutWithScrollbars();
	}
	else
	{
		LayoutTextArea();
	}
}

void UI_TextArea::ProcessInputCommand(const InputCmd& cmd)
{

	for (auto& textbox : _textboxes)
	{
		textbox->ProcessInputCommand(cmd);
	}

    if (HasFocus())
    {
		_eventCurrentFocusObserver->SetEventData(_name);
        auto oldPos = _cursorPosition;
        switch (cmd.type)
        {
            case INPUT_CURSOR_LEFT:
                _highlighting = cmd.flags == FLAG_HIGHLIGHT;
                _cursorPosition.column = _textboxes.at(_cursorPosition.line)->GetCursorPosition();
                break;

            case INPUT_CURSOR_RIGHT:
                _highlighting = cmd.flags == FLAG_HIGHLIGHT;
                _cursorPosition.column = _textboxes.at(_cursorPosition.line)->GetCursorPosition();
                break;

            case INPUT_CURSOR_UP:
                _highlighting = cmd.flags == FLAG_HIGHLIGHT;
                UpdateCursorPosition(-1, 0);
                break;

            case INPUT_CURSOR_DOWN:
                _highlighting = cmd.flags == FLAG_HIGHLIGHT;
                UpdateCursorPosition(1, 0);
                break;

            case INPUT_CURSOR_MOVE:
                _highlighting = cmd.flags == FLAG_HIGHLIGHT;
                _cursorPosition = _hoverPosition;
                _textboxes.at(_cursorPosition.line)->SetCursorPosition(_cursorPosition.column);
                if (!_highlighting)
                {
                    _highlightPosition = _cursorPosition;
                }
                break;

            case INPUT_CURSOR_DRAG:
                if (!_highlighting)
                {
                    _highlightPosition = _cursorPosition;
                }
                _highlighting = true;
                _cursorPosition = _hoverPosition;
                break;

            case INPUT_INSERT:
                if (_textboxes.empty())
                {
                    _textboxes.push_back(std::make_unique<UI_Textbox>(_config, TextboxName(), _eventTextboxFocus.Subscribe(), std::make_unique<BarCursorStrategy>(_config)));
                }
                if (_highlighting)
                {
                    DeleteHighlightedText();
                    _highlighting = false;
                }
                _cursorPosition.column = _textboxes.at(_cursorPosition.line)->GetCursorPosition();
                break;

            case INPUT_DELETE:
                if (_highlighting)
                {
                    DeleteHighlightedText();
                    _highlighting = false;
                }
                else
                {
                    HandleDeleteCommand(cmd);
                }
                break;

            case INPUT_CURSOR_HOME:
                _highlighting = cmd.flags == FLAG_HIGHLIGHT;
                _cursorPosition.column = 0;
                break;

            case INPUT_CURSOR_END:
                _highlighting = cmd.flags == FLAG_HIGHLIGHT;
                _cursorPosition.column = _textboxes.at(_cursorPosition.line)->GetText().length();
                break;

            case INPUT_SCROLL_DOWN:
                _scrollOffset.line++;
                UpdateScrollOffset();
                break;

            case INPUT_SCROLL_UP:
                if (_scrollOffset.line > 0)
                {
                    _scrollOffset.line--;
                }
                break;

            case INPUT_CURSOR_PAGEUP:
                UpdateCursorPosition(-1, 0);
                break;

            case INPUT_CURSOR_PAGEDOWN:
                UpdateCursorPosition(1, 0);
                break;

            case INPUT_NEWLINE:
                HandleNewlineCommand();
                break;

            case INPUT_CURSOR_COPY:
                _copyBuffer = GetHighlightedText();
                break;

            case INPUT_CURSOR_CUT:
                _copyBuffer = GetHighlightedText();
                DeleteHighlightedText();
                _highlighting = false;
                ClearAllHighlighting();
                break;

            case INPUT_CURSOR_PASTE:
                if (_highlighting)
                {
                    DeleteHighlightedText();
                    _highlighting = false;
                    ClearAllHighlighting();
                }
                InsertText(_copyBuffer);
                _highlighting = false;
                break;
        }

        if (cmd.type == INPUT_INSERT || cmd.type == INPUT_CURSOR_LEFT || cmd.type == INPUT_CURSOR_RIGHT || cmd.type == INPUT_CURSOR_DOWN || cmd.type == INPUT_CURSOR_UP || cmd.type == INPUT_CURSOR_HOME || cmd.type == INPUT_CURSOR_END || cmd.type == INPUT_NEWLINE)
        {
            ScrollCursorIntoView();
        }

        if (_hoverPosition.line >= _textboxes.size())
        {
            _hoverPosition.line = _textboxes.size() - 1;
        }

        if (_highlighting)
        {
            ClearAllHighlighting();
            HighlightText();
        }
        else
        {
            _highlightPosition = _cursorPosition;
        }

        if (oldPos != _cursorPosition)
        {
            _textboxes.at(_cursorPosition.line)->SetCursorPosition(_cursorPosition.column);
            _textboxes.at(_cursorPosition.line)->SetFocus();
        }
    }
	else
	{
		_eventTextboxFocus.SetEventData("");
	}
}


void UI_TextArea::SetLayoutDimensions(const LayoutDimensions& dim)
{
	_layoutDimensions = dim;
	_layoutDimensions.height -= _config.fontSize;
	if (_lineNumberingStrategy)
	{
		_layoutDimensions.width -= _config.fontSize * 2;
	}
	NormalizeScrollOffset();
}

void UI_TextArea::SetText(const std::string& text)
{
	_text = text;
	_textboxes.clear();
	auto lines = GetLinesBySplitor(text, '\n');
	for (auto line : lines)
	{
		_textboxes.push_back(std::make_unique<UI_Textbox>(_config, TextboxName(), _eventTextboxFocus.Subscribe(), std::make_unique<BarCursorStrategy>(_config)));
		_textboxes.back()->SetText(line);
	}
}

const std::string& UI_TextArea::GetText()
{
	_text = "";
	for (auto& tb : _textboxes)
	{
		_text += tb->GetText() + '\n';
	}
	return _text;
}

void UI_TextArea::LayoutWithScrollbars()
{

	//_lineNumberingStrategy->LayoutLineNumbering(index);
	CLAY(
		CLAY_ID_LOCAL("LINENUMBERS_PARENT"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)_config.fontSize * 2), .height = CLAY_SIZING_GROW(0)},  .layoutDirection = CLAY_LEFT_TO_RIGHT }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		CLAY(
			CLAY_ID_LOCAL("LINENUMBERS"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},  .layoutDirection = CLAY_TOP_TO_BOTTOM }),
			CLAY_RECTANGLE({ .color = _config.colors.background })
		)
		{
			if (_lineNumberingStrategy)
			{
				uint16_t maxLines = GetViewPortHeight();
				uint16_t start_line = _scrollOffset.line;

				std::array<uint16_t, 256> linesToDisplay{ 0 };
				std::iota(linesToDisplay.begin(), linesToDisplay.end(), 0);
				auto lineView = linesToDisplay | std::views::drop(start_line) | std::views::take(maxLines);
				for (auto iter : lineView)
				{
					if (iter > _textboxes.size()-1)
					{
						break;
					}
					_lineNumberingStrategy->LayoutLineNumbering(iter);
				}
			}
		}

		//do everything else
		CLAY(
			CLAY_ID_LOCAL("TEXTAREA_CONTAINER_VSCROLL"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(_layoutDimensions.width), .height = CLAY_SIZING_FIXED(_layoutDimensions.height)},  .layoutDirection = CLAY_LEFT_TO_RIGHT }),
			CLAY_RECTANGLE({ .color = _config.colors.background })
		)
		{
			CLAY(
				CLAY_ID_LOCAL("TEXTAREA_CONTAINER_HSCROLL"),
				CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0,_layoutDimensions.width - _config.fontSize), .height = CLAY_SIZING_GROW(0)},  .layoutDirection = CLAY_TOP_TO_BOTTOM }),
				CLAY_RECTANGLE({ .color = _config.colors.background })
			)
			{
				LayoutTextArea();

				CLAY(
					CLAY_ID_LOCAL("TEXTAREA_CONTAINER_HSCROLL_PAD"),
					CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(_layoutDimensions.height)},  .layoutDirection = CLAY_LEFT_TO_RIGHT }),
					CLAY_RECTANGLE({ .color = _config.colors.background })
				)
				{
					LayoutScrollbarH();
				}
			}
			LayoutScrollbarV();
		}
	}
}

void UI_TextArea::LayoutTextArea()
{
	CLAY(
		CLAY_ID_LOCAL("TEXTAREA"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}, .childAlignment = {.x=CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		if (Clay_Hovered() && IsMouseButtonDown(0))
		{
			//observe focus
			_eventCurrentFocusObserver->SetEventData(_name);
		}

		uint16_t maxLines = GetViewPortHeight();
		uint16_t start_line = _scrollOffset.line;

		std::array<uint16_t, 256> linesToDisplay{ 0 };
		std::iota(linesToDisplay.begin(), linesToDisplay.end(), 0);
		auto lineView = linesToDisplay | std::views::drop(start_line) | std::views::take(maxLines);

		for (auto iter : lineView)
		{
			CreateLine(iter);
		}
	}
}

void UI_TextArea::LayoutScrollbarH()
{
	auto maxChars = GetViewPortWidth();
	if (maxChars <= 0)
	{
		CLAY(
			CLAY_ID_LOCAL("SCROLLHDUMMY"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)}, }),
			CLAY_RECTANGLE({ .color = _config.colors.background })
		)
		{
		}
		return;
	}

	auto longestElm = std::max_element(_textboxes.begin(), _textboxes.end(), [](const auto& lhs, const auto& rhs) { return lhs->GetText().length() < rhs->GetText().length(); });
	auto maxLength = longestElm->get()->GetText().length();

	float pageCount = maxLength / maxChars;
	bool canScroll = pageCount >= 1;

	if (canScroll)
	{
		LayoutDimensions layoutDimensions = { (_layoutDimensions.width - _config.fontSize), (float)_config.fontSize };
		_scrollBarH->SetLayoutDimensions(layoutDimensions);
		_scrollBarH->SetMin(0);
		_scrollBarH->SetMax(maxLength);
		_scrollBarH->SetValue(_scrollOffset.column);

		_scrollBarH->SetStep(maxChars);

		_scrollBarH->Layout();

		_scrollOffset.column = _scrollBarH->GetValue();
	}
	else
	{
		CLAY(
			CLAY_ID_LOCAL("SCROLLHDUMMY"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)}, }),
			CLAY_RECTANGLE({ .color = _config.colors.background })
		)
		{
		}
	}
}

void UI_TextArea::LayoutScrollbarV()
{
	auto maxLines = _layoutDimensions.height / _config.fontSize;
	if (maxLines <= 0)
	{
		CLAY(
			CLAY_ID_LOCAL("SCROLLVDUMMY"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)_config.fontSize), .height = CLAY_SIZING_GROW(0)}, }),
			CLAY_RECTANGLE({ .color = _config.colors.background })
		)
		{
		}
		return;
	}
	float pageCount = _textboxes.size() / maxLines;
	bool canScroll = pageCount >= 1;

	if (canScroll)
	{
		_scrollBarV->SetLayoutDimensions(_layoutDimensions);
		_scrollBarV->SetMin(0);
		_scrollBarV->SetMax(_textboxes.size());
		_scrollBarV->SetValue(_scrollOffset.line);

		_scrollBarV->SetStep(maxLines);

		_scrollBarV->Layout();

		_scrollOffset.line = _scrollBarV->GetValue();
	}
	else
	{
		CLAY(
			CLAY_ID_LOCAL("SCROLLVDUMMY"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)_config.fontSize), .height = CLAY_SIZING_GROW(0)}, }),
			CLAY_RECTANGLE({ .color = _config.colors.background })
		)
		{
		}
	}
}

void UI_TextArea::UpdateCursorPosition(int lineDelta, int columnDelta)
{
    _cursorPosition.line = SafeClampLow(_cursorPosition.line + lineDelta, 0, (int)_textboxes.size() - 1);
    _cursorPosition.column = SafeClampLow(_cursorPosition.column + columnDelta, 0, (int)_textboxes.at(_cursorPosition.line)->GetText().length());
}

void UI_TextArea::UpdateScrollOffset()
{
    uint16_t maxLines = _layoutDimensions.height / _config.fontSize;
    _scrollOffset.line = SafeClampLow(_scrollOffset.line, (uint16_t)0u, (uint16_t)(_textboxes.size() - maxLines));

    auto maxChars = GetViewPortWidth();
    auto longestElm = std::max_element(_textboxes.begin(), _textboxes.end(), [](const auto& lhs, const auto& rhs) { return lhs->GetText().length() < rhs->GetText().length(); });
    auto maxLength = longestElm->get()->GetText().length();
    _scrollOffset.column = SafeClampLow(_scrollOffset.column, (uint16_t)0u, (uint16_t)(maxLength - maxChars));
}

void UI_TextArea::ScrollCursorIntoView()
{
    uint16_t maxLines = _layoutDimensions.height / _config.fontSize;
    if (_cursorPosition.line >= _scrollOffset.line + maxLines)
    {
        _scrollOffset.line = _cursorPosition.line - maxLines + 1;
    }
    else if (_cursorPosition.line < _scrollOffset.line)
    {
        _scrollOffset.line = _cursorPosition.line;
    }

    auto maxChars = GetViewPortWidth();
    if (_cursorPosition.column >= _scrollOffset.column + maxChars)
    {
        _scrollOffset.column = _cursorPosition.column - maxChars + 1;
    }
    else if (_cursorPosition.column < _scrollOffset.column)
    {
        _scrollOffset.column = _cursorPosition.column;
    }

    UpdateScrollOffset();
}

void UI_TextArea::NormalizeScrollOffset()
{
	uint16_t maxLines = _layoutDimensions.height / _config.fontSize;
	int16_t maxOffset = _textboxes.size() - maxLines;
	if (_scrollOffset.line > maxOffset)
	{
		if (maxOffset < 0)
		{
			_scrollOffset.line = 0;
		}
		else
		{
			_scrollOffset.line = maxOffset;
		}
	}

	auto maxChars = GetViewPortWidth();
	auto longestElm = std::max_element(_textboxes.begin(), _textboxes.end(), [](const auto& lhs, const auto& rhs) { return lhs->GetText().length() < rhs->GetText().length(); });
	auto maxLength = longestElm->get()->GetText().length();
	int16_t maxOffsetH = maxLength - std::floor(maxChars);

	if (_scrollOffset.column > maxOffsetH && maxOffsetH > 0)
	{
		if (maxOffsetH < 0)
		{
			_scrollOffset.column = 0;
		}
		else
		{
			_scrollOffset.column = maxOffsetH;
		}
	}
}

uint16_t UI_TextArea::GetViewPortWidth()
{
	return std::max(0.0f, (_layoutDimensions.width) - _config.fontSize) / (_config.fontSize / 2);
}

uint16_t UI_TextArea::GetViewPortHeight()
{
	return std::max(0.0f, _layoutDimensions.height) / _config.fontSize;
}


void UI_TextArea::CreateLine(size_t index)
{
	CLAY(
		CLAY_IDI_LOCAL("LINE", index),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)}, .layoutDirection = CLAY_LEFT_TO_RIGHT }),
		//CLAY_BORDER_OUTSIDE({ .width = borderSize, .color = _config.colors.highlight }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		if (index < _textboxes.size())
		{
			auto maxChars = GetViewPortWidth();
			uint16_t start_char = _scrollOffset.column;
			auto maxLength = std::min((int16_t)(_textboxes.at(index)->GetText().length()), (int16_t)(maxChars + start_char));

			if (maxLength < 0)
			{
				maxLength = 0;
			}

			_textboxes.at(index)->Layout(start_char, maxLength);

			if (Clay_Hovered())
			{
				_hoverPosition.line = index;
				_hoverPosition.column = _textboxes.at(index)->GetHoverPosition();
			}
		}
	}
}

std::string UI_TextArea::GetHighlightedText()
{
	std::string highlightedText = "";

	//mark each highlighted character
	auto startLine = std::min(_cursorPosition.line, _highlightPosition.line);
	auto endLine = std::max(_cursorPosition.line, _highlightPosition.line);

	auto startColumn = startLine == _cursorPosition.line ? _cursorPosition.column : _highlightPosition.column;
	auto endColumn = endLine == _cursorPosition.line ? _cursorPosition.column : _highlightPosition.column;

	if (startLine == endLine)
	{
		highlightedText = _textboxes.at(startLine)->GetHighlightedText();
	}
	else
	{
		for (auto iter = startLine; iter <= endLine; iter++)
		{
			highlightedText += _textboxes.at(iter)->GetHighlightedText() +"\n";
		}
		highlightedText.pop_back();
	}

	return highlightedText;
}

void UI_TextArea::DeleteHighlightedText()
{
	//mark each highlighted character
	auto startLine = std::min(_cursorPosition.line, _highlightPosition.line);
	auto endLine = std::max(_cursorPosition.line, _highlightPosition.line);

	auto startColumn = startLine == _cursorPosition.line ? _cursorPosition.column : _highlightPosition.column;
	auto endColumn = endLine == _cursorPosition.line ? _cursorPosition.column : _highlightPosition.column;
	
	if (startLine == endLine)
	{
		_textboxes.at(startLine)->DeleteHighlightedText();
	}
	else
	{
		_textboxes.at(startLine)->DeleteHighlightedText();
		_textboxes.at(endLine)->DeleteHighlightedText();
		auto startText = _textboxes.at(startLine)->GetText();
		auto endText = _textboxes.at(endLine)->GetText();
		_textboxes.at(startLine)->SetText(startText.substr(0, startColumn) + endText);
		_textboxes.erase(_textboxes.begin() + startLine + 1, _textboxes.begin() + endLine + 1);
		_cursorPosition.line = startLine;
		_cursorPosition.column = startColumn;
	}
}

void UI_TextArea::InsertText(const std::string& text)
{
	auto lines = GetLinesBySplitor(text, '\n');

	auto currentText = _textboxes.at(_cursorPosition.line)->GetText();
	auto currentLine = currentText.substr(0, _cursorPosition.column);
	auto nextLine = currentText.substr(_cursorPosition.column);
	
	lines.at(0) = currentLine + lines.at(0);
	lines.at(lines.size()-1) = lines.at(lines.size()-1) + nextLine;

	_textboxes.at(_cursorPosition.line)->SetText(lines[0]);

	for (auto& line : std::views::drop(lines, 1))
	{
		_textboxes.insert(_textboxes.begin() + _cursorPosition.line + 1, std::make_unique<UI_Textbox>(_config, TextboxName(), _eventTextboxFocus.Subscribe(), std::make_unique<BarCursorStrategy>(_config)));
		_textboxes.at(_cursorPosition.line + 1)->SetText(line);
		_cursorPosition.line++;
	}
}
void UI_TextArea::HandleDeleteCommand(const InputCmd& cmd)
{
	if (cmd.flags == FLAG_DELETE_FWD)
	{
		if (!_textboxes.at(_cursorPosition.line)->CursorAtEnd())
		{
			//noop
		}
		else if (_cursorPosition.line < _textboxes.size() - 1)
		{
			auto nextLine = _textboxes.at(_cursorPosition.line + 1)->GetText();
			_textboxes.at(_cursorPosition.line)->InsertText(nextLine);
			_textboxes.erase(_textboxes.begin() + _cursorPosition.line + 1);
		}
	}
	else if (cmd.flags == FLAG_DELETE_BWD)
	{
		if (!_textboxes.at(_cursorPosition.line)->CursorAtStart())
		{
			//noop
		}
		else if (_cursorPosition.line > 0)
		{
			auto line = _textboxes.at(_cursorPosition.line)->GetText();
			auto prevLine = _textboxes.at(_cursorPosition.line - 1)->GetText();
			_cursorPosition.column = prevLine.length();
			_textboxes.at(_cursorPosition.line - 1)->SetCursorPosition(prevLine.length());
			_textboxes.at(_cursorPosition.line - 1)->InsertText(line);
			_textboxes.erase(_textboxes.begin() + _cursorPosition.line);
			_cursorPosition.line--;
		}
	}
}

void UI_TextArea::HandleNewlineCommand()
{
	if (_highlighting)
	{
		DeleteHighlightedText();
		_highlighting = false;
	}
	auto position = _textboxes.at(_cursorPosition.line)->GetCursorPosition();
	auto line = _textboxes.at(_cursorPosition.line)->GetText();
	_textboxes.insert(_textboxes.begin() + _cursorPosition.line + 1, std::make_unique<UI_Textbox>(_config, TextboxName(), _eventTextboxFocus.Subscribe(), std::make_unique<BarCursorStrategy>(_config)));

	if (line.length() > position)
	{
		_textboxes.at(_cursorPosition.line + 1)->InsertText(line.substr(position, line.length() - position));
	}
	else
	{
		_textboxes.at(_cursorPosition.line + 1)->InsertText("");
	}
	_textboxes.at(_cursorPosition.line)->SetText(line.substr(0, position));
	_cursorPosition.line++;
	_cursorPosition.column = 0;
}

void UI_TextArea::ClearAllHighlighting()
{
	for (auto& textbox : _textboxes)
	{
		textbox->SetHighlighting(false);
	}
}

void UI_TextArea::HighlightText()
{
	auto startLine = std::min(_cursorPosition.line, _highlightPosition.line);
	auto endLine = std::max(_cursorPosition.line, _highlightPosition.line);

	auto startColumn = startLine == _cursorPosition.line ? _cursorPosition.column : _highlightPosition.column;
	auto endColumn = endLine == _cursorPosition.line ? _cursorPosition.column : _highlightPosition.column;

	if (startLine == endLine)
	{
		_textboxes.at(startLine)->SetHighlighting(true);
		_textboxes.at(startLine)->SetHighlightingPosition(_cursorPosition.column, _highlightPosition.column);
	}
	else
	{
		for (auto iter = startLine; iter <= endLine; iter++)
		{
			if (iter < _textboxes.size() && iter >= 0)
			{
				if (iter == startLine)
				{
					_textboxes.at(iter)->SetHighlightingPosition(startColumn, HIGHLIGHT_END);
				}
				else if (iter == endLine)
				{
					_textboxes.at(iter)->SetHighlightingPosition(endColumn, 0);
				}
				else
				{
					_textboxes.at(iter)->SetHighlightingPosition(0, HIGHLIGHT_END);
				}
				_textboxes.at(iter)->SetHighlighting(true);
			}
		}
	}
}







