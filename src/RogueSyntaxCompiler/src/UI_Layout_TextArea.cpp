#include "UI_Layout_TextArea.h"
#include <iostream>

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

	_textboxes.push_back(std::make_unique<UI_Textbox>(_config, TextboxName(), _eventTextboxFocus.Subscribe(), std::make_unique<BarCursorStrategy>(_config)));
}

void UI_TextArea::Layout()
{
	_hasFocus = _eventCurrentFocusObserver->GetEventData() == _name;
	if (!_highlighting)
	{
		for (auto& textbox : _textboxes)
		{
			textbox->SetHighlighting(false);
		}
	}

	LayoutTextArea();
}

void UI_TextArea::ProcessInputCommand(const InputCmd& cmd)
{
	if (_hasFocus)
	{
		auto oldPos = _cursorPosition;
		switch (cmd.type)
		{
			case INPUT_CURSOR_LEFT:
			{
				_highlighting = cmd.flags == FLAG_HIGHLIGHT;
				_textboxes.at(_cursorPosition.line)->ProcessInputCommand(cmd);
				_cursorPosition.column = _textboxes.at(_cursorPosition.line)->GetCursorPosition();
				break;
			}
			case INPUT_CURSOR_RIGHT:
			{
				_highlighting = cmd.flags == FLAG_HIGHLIGHT;
				_textboxes.at(_cursorPosition.line)->ProcessInputCommand(cmd);
				_cursorPosition.column = _textboxes.at(_cursorPosition.line)->GetCursorPosition();
				break;
			}
			case INPUT_CURSOR_UP:
			{
				_highlighting = cmd.flags == FLAG_HIGHLIGHT;
				if (_cursorPosition.line > 0)
				{
					_cursorPosition.column = _textboxes.at(_cursorPosition.line)->GetCursorPosition();
					_cursorPosition.line--;
					_cursorPosition.column = std::min((uint16_t)_textboxes.at(_cursorPosition.line)->GetText().length(), _cursorPosition.column);
				}
				break;
			}
			case INPUT_CURSOR_DOWN:
			{
				_highlighting = cmd.flags == FLAG_HIGHLIGHT;
				if (_cursorPosition.line < _textboxes.size() - 1)
				{
					_cursorPosition.column = _textboxes.at(_cursorPosition.line)->GetCursorPosition();
					_cursorPosition.line++;
					_cursorPosition.column = std::min((uint16_t)_textboxes.at(_cursorPosition.line)->GetText().length(), _cursorPosition.column);
				}
				break;
			}
			case INPUT_CURSOR_MOVE:
			{
				_highlighting = cmd.flags == FLAG_HIGHLIGHT;

				_cursorPosition.line = _hoverPosition.line;
				_cursorPosition.column = _hoverPosition.column;
				_textboxes.at(_cursorPosition.line)->SetCursorPosition(_cursorPosition.column);

				_textboxes.at(_cursorPosition.line)->ProcessInputCommand(cmd);

				if (!_highlighting)
				{
					_highlightPosition.column = _cursorPosition.column;
					_highlightPosition.line = _cursorPosition.line;
				}

				break;
			}
			case INPUT_CURSOR_DRAG:
			{
				if (!_highlighting)
				{
					_highlightPosition.column = _cursorPosition.column;
					_highlightPosition.line = _cursorPosition.line;
				}
				_highlighting = true;
				_cursorPosition.line = _hoverPosition.line;
				_cursorPosition.column = _hoverPosition.column;

				_textboxes.at(_cursorPosition.line)->ProcessInputCommand(cmd);
				break;
			}
			case INPUT_INSERT:
			{
				if (_textboxes.size() == 0)
				{
					_textboxes.push_back(std::make_unique<UI_Textbox>(_config, TextboxName(), _eventTextboxFocus.Subscribe(), std::make_unique<BarCursorStrategy>(_config)));
				}

				if (_highlighting)
				{
					DeleteHighlightedText();
					_highlighting = false;
				}
				
				_textboxes.at(_cursorPosition.line)->ProcessInputCommand(cmd);

				break;
			}
			case INPUT_DELETE:
			{
				if (_highlighting)
				{
					DeleteHighlightedText();
					_highlighting = false;
				}
				else
				{
					if (cmd.flags == FLAG_DELETE_FWD)
					{

						if (!_textboxes.at(_cursorPosition.line)->CursorAtEnd())
						{
							_textboxes.at(_cursorPosition.line)->ProcessInputCommand(cmd);
						}
						else
						{
							if (_cursorPosition.line < _textboxes.size() - 1)
							{
								auto nextLine = _textboxes.at(_cursorPosition.line + 1)->GetText();
								_textboxes.at(_cursorPosition.line)->InsertText(nextLine);
								_textboxes.erase(_textboxes.begin() + _cursorPosition.line + 1);
							}
						}
					}
					else if (cmd.flags == FLAG_DELETE_BWD)
					{
						if (!_textboxes.at(_cursorPosition.line)->CursorAtStart())
						{
							_textboxes.at(_cursorPosition.line)->ProcessInputCommand(cmd);
						}
						else
						{
							if (_cursorPosition.line > 0)
							{
								auto line = _textboxes.at(_cursorPosition.line)->GetText();
								auto prevLine = _textboxes.at(_cursorPosition.line - 1)->GetText();
								_cursorPosition.column = prevLine.length();
								_textboxes.at(_cursorPosition.line - 1)->InsertText(line);
								_textboxes.erase(_textboxes.begin() + _cursorPosition.line);
								_cursorPosition.line--;
							}
						}
					}
				}
				break;
			}
			case INPUT_CURSOR_HOME:
			{
				_highlighting = cmd.flags == FLAG_HIGHLIGHT;
				_textboxes.at(_cursorPosition.line)->ProcessInputCommand(cmd);
				break;
			}
			case INPUT_CURSOR_END:
			{
				_highlighting = cmd.flags == FLAG_HIGHLIGHT;
				_textboxes.at(_cursorPosition.line)->ProcessInputCommand(cmd);
				break;
			}
			case INPUT_SCROLL_DOWN:
			{
				_scrollPosition.line++;
				break;
			}
			case INPUT_SCROLL_UP:
			{
				if (_scrollPosition.line > 0)
				{
					_scrollPosition.line--;
				}
				break;
			}
			case INPUT_CURSOR_PAGEUP:
			{
				if (_cursorPosition.line > 0)
				{
					_cursorPosition.line--;
				}
				break;
			}
			case INPUT_CURSOR_PAGEDOWN:
			{
				_cursorPosition.line++;
				if (_cursorPosition.line >= _textboxes.size())
				{
					_cursorPosition.line = _textboxes.size() - 1;
				}
				break;
			}
			case INPUT_NEWLINE:
			{
				if (_highlighting)
				{
					DeleteHighlightedText();
					_highlighting = false;
				}
				auto position = _textboxes.at(_cursorPosition.line)->GetCursorPosition();
				auto line = _textboxes.at(_cursorPosition.line)->GetText();
				//add textbox
				_textboxes.insert(_textboxes.begin() + _cursorPosition.line + 1, std::make_unique<UI_Textbox>(_config, TextboxName(), _eventTextboxFocus.Subscribe(), std::make_unique<BarCursorStrategy>(_config)));
				_textboxes.at(_cursorPosition.line + 1)->InsertText(line.substr(position));
				_textboxes.at(_cursorPosition.line)->SetText(line.substr(0, position));
				_cursorPosition.line++;
				_cursorPosition.column = 0;

				break;
			}
			case INPUT_CURSOR_COPY:
			{
				GetHighlightedText();
				break;
			}
			case INPUT_CURSOR_CUT:
			{
				GetHighlightedText();
				DeleteHighlightedText();
				_highlighting = false;
				break;
			}
			case INPUT_CURSOR_PASTE:
			{
				InsertText("");
				_highlighting = false;
				break;
			}
		}
		if (_highlighting)
		{
			//clear all highlighting
			for (auto& textbox : _textboxes)
			{
				textbox->SetHighlighting(false);
			}

			//mark each highlighted character
			auto startLine = std::min(_cursorPosition.line, _highlightPosition.line);
			auto endLine = std::max(_cursorPosition.line, _highlightPosition.line);

			auto startColumn = std::min(_cursorPosition.column, _highlightPosition.column);
			auto endColumn = std::max(_cursorPosition.column, _highlightPosition.column);

			if (startLine == endLine)
			{
				_textboxes.at(startLine)->SetHighlighting(true);
				_textboxes.at(startLine)->SetHighlightingPosition(startColumn, endColumn);
			}
			else
			{
				for (auto iter = startLine; iter <= endLine; iter++)
				{
					if (iter == startLine)
					{
						_textboxes.at(iter)->SetHighlightingPosition(startColumn, HIGHLIGHT_END);
					}
					else if (iter == endLine)
					{
						_textboxes.at(iter)->SetHighlightingPosition(0, endColumn);
					}
					else
					{
						_textboxes.at(iter)->SetHighlightingPosition(0, HIGHLIGHT_END);
					}

					_textboxes.at(iter)->SetHighlighting(true);
				}
			}
		}

		if (oldPos != _cursorPosition)
		{
			_textboxes.at(_cursorPosition.line)->SetCursorPosition(_cursorPosition.column);
			_textboxes.at(_cursorPosition.line)->SetFocus();
		}
		else
		{
			
			//update the last cursor position
			if (!_highlighting)
			{
				_highlightPosition.column = _cursorPosition.column;
				_highlightPosition.line = _cursorPosition.line;
				//_textboxes.at(_cursorPosition.line)
			}	
		}
	}
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

void UI_TextArea::LayoutTextArea()
{
	CLAY(
		CLAY_ID_LOCAL("TEXTAREA"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},  .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		if (Clay_Hovered() && IsMouseButtonDown(0))
		{
			//observe focus
			_eventCurrentFocusObserver->SetEventData(_name);
			_hasFocus = true;
		}

		for (size_t i = 0; i < _textboxes.size(); i++)
		{
			CreateLine(i);
		}
	}
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
		_lineNumberingStrategy->LayoutLineNumbering(index);
		_textboxes.at(index)->Layout();

		if (Clay_Hovered())
		{
			_hoverPosition.line = index;
			_hoverPosition.column = _textboxes.at(index)->GetHoverPosition();
		}
	}
}

std::string UI_TextArea::GetHighlightedText()
{
	return std::string();
}

void UI_TextArea::DeleteHighlightedText()
{
}

void UI_TextArea::InsertText(const std::string& text)
{
}







