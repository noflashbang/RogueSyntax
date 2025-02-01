#include "UI_Layout_Textbox.h"

UI_Textbox::UI_Textbox(const UIConfig& config, const std::string name, std::shared_ptr<UIEventObserver<std::string>> focusChanged, std::unique_ptr<ICursorStrategy> cursorType) 
	: _config(config), _eventCurrentFocusObserver(focusChanged), _name(name), _cursorStrategy(std::move(cursorType))
{
	_text = "";
}

void UI_Textbox::Layout()
{
	if (_cursorStrategy != nullptr)
	{
		_cursorStrategy->Update(GetFrameTime());
	}

	LayoutTextbox(0, _text.length());
}

void UI_Textbox::Layout(uint16_t offset, uint16_t length)
{
	if (_cursorStrategy != nullptr)
	{
		_cursorStrategy->Update(GetFrameTime());
	}

	LayoutTextbox(offset, length);
}

void UI_Textbox::ProcessInputCommand(const InputCmd& cmd)
{
	if (HasFocus())
	{
			_eventCurrentFocusObserver->SetEventData(_name);
			switch (cmd.type)
			{
				case INPUT_CURSOR_LEFT:
				{
					_highlighting = cmd.flags == FLAG_HIGHLIGHT;
					if (_cursorPosition > 0)
					{
						_cursorPosition--;
					}
					break;
				}
				case INPUT_CURSOR_RIGHT:
				{
					_highlighting = cmd.flags == FLAG_HIGHLIGHT;
					if (_cursorPosition < _text.length())
					{
						_cursorPosition++;
					}
					break;
				}
				case INPUT_CURSOR_MOVE:
				{
					_highlighting = cmd.flags == FLAG_HIGHLIGHT;
					_cursorPosition = _hoverPosition;

					if (!_highlighting)
					{
						_highlightPosition = _cursorPosition;
					}

					break;
				}
				case INPUT_CURSOR_DRAG:
				{
					if (!_highlighting)
					{
						_highlightPosition = _cursorPosition;
					}
					_highlighting = true;
					_cursorPosition = _hoverPosition;
					break;
				}
				case INPUT_INSERT:
				{
					if (_highlighting)
					{
						DeleteHighlightedText();
						_highlighting = false;
					}
					char character = (char)cmd.flags;
					if (_cursorPosition >= _text.length())
					{
						_text.push_back(character);
						_cursorPosition = _text.length();
					}
					else
					{
						_text.insert(_cursorPosition, 1, character);
						_cursorPosition++;
					}
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
							if (_cursorPosition < _text.length())
							{
								_text.erase(_cursorPosition, 1);
							}
						}
						else if (cmd.flags == FLAG_DELETE_BWD)
						{
							if (_cursorPosition > 0)
							{
								_cursorPosition--;
								_text.erase(_cursorPosition, 1);
							}
						}
					}
					break;
				}
				case INPUT_CURSOR_HOME:
				{
					_highlighting = cmd.flags == FLAG_HIGHLIGHT;
					_cursorPosition = 0;
					break;
				}
				case INPUT_CURSOR_END:
				{
					_highlighting = cmd.flags == FLAG_HIGHLIGHT;
					_cursorPosition = _text.length();
					break;
				}
				case INPUT_NEWLINE:
				{
					//callback?
					break;
				}
				case INPUT_CURSOR_COPY:
				{
					_copyBuffer = GetHighlightedText();
					break;
				}
				case INPUT_CURSOR_CUT:
				{
					_copyBuffer = GetHighlightedText();
					DeleteHighlightedText();
					_highlighting = false;
					break;
				}
				case INPUT_CURSOR_PASTE:
				{
					if (_highlighting)
					{
						DeleteHighlightedText();
						_highlighting = false;
					}
					InsertText(_copyBuffer);
					_highlighting = false;
					break;
				}
		}

		//update the last cursor position
		if (!_highlighting)
		{
			_highlightPosition = _cursorPosition;
		}

	}
}

void UI_Textbox::SetFocus()
{
	_eventCurrentFocusObserver->SetEventData(_name);
	if (!_highlighting)
	{
		_highlightPosition = _cursorPosition;
	}
}
void UI_Textbox::SetHighlightingPosition(uint16_t cursor, uint16_t hightlight)
{
	if (hightlight == HIGHLIGHT_END)
	{
		_highlightPosition = _text.size();
		_cursorPosition = cursor;
	}
	else
	{
		_highlightPosition = hightlight;
		_cursorPosition = cursor;
	}
};

void UI_Textbox::LayoutCursor(size_t index)
{
	if (_cursorStrategy != nullptr)
	{
		if (HasFocus() && (_cursorPosition == index))
		{
			_cursorStrategy->LayoutCursor();
		}
	}
}

void UI_Textbox::LayoutTextbox(uint16_t offset, uint16_t length)
{
	CLAY(
		CLAY_ID_LOCAL("TEXTBOX"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
		//CLAY_BORDER_OUTSIDE({ .width = borderSize, .color = _config.colors.highlight }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		if (Clay_Hovered() && IsMouseButtonDown(0))
		{
			//observe focus
			_eventCurrentFocusObserver->SetEventData(_name);
		}

		if (offset >= length)
		{
			CreatePlaceHolderChar(0, &_cursorPlaceholder);
		}
		else
		{
			for (auto index = offset; index < length; index++)
			{
				char* character = (char*)_text.data() + index;
				if (character != nullptr && *character != '\0' && *character != '\n')
				{
					CreateChar(index, character);
				}
			}

			//for drawing the cursor at the end of the line - also handles clicks at the end of the line
			CreatePlaceHolderChar(length, &_cursorPlaceholder);
		}
	}
}
void UI_Textbox::CreateChar(size_t index, const char* character)
{
	auto strContent = Clay_String{ .length = 1, .chars = character };
	Clay_Color textColor = _config.colors.text;
	Clay_Color backgroundColor = _config.colors.background;

	if (_highlighting)
	{
		auto start = std::min(_cursorPosition, _highlightPosition);
		auto end = std::max(_cursorPosition, _highlightPosition);
		
		bool between = start <= index && index < end;

		if(between)
		{
			backgroundColor = _config.colors.foreground;
			textColor = _config.colors.accentText;
		}
	}

	CLAY(
		CLAY_IDI_LOCAL("COL", index),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)(_config.fontSize / 2)), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
		CLAY_RECTANGLE({ .color = backgroundColor })
	)
	{
		LayoutCursor(index);
		
		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = textColor, .fontId = _config.fontId, .fontSize = _config.fontSize }));
		if (Clay_Hovered())
		{
			_hoverPosition = index;
		}
	}
}
void UI_Textbox::CreatePlaceHolderChar(size_t index, const char* character)
{
	auto strContent = Clay_String{ .length = 1, .chars = character };
	Clay_Color textColor = _config.colors.text;
	Clay_Color backgroundColor = _config.colors.background;

	CLAY(
		CLAY_IDI_LOCAL("COL", index),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
		CLAY_RECTANGLE({ .color = backgroundColor })
	)
	{
		LayoutCursor(index);

		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = textColor, .fontId = _config.fontId, .fontSize = _config.fontSize }));
		if (Clay_Hovered())
		{
			_hoverPosition = index;
		}
	}
}

std::string UI_Textbox::GetHighlightedText()
{
	auto startColumn = std::min(_cursorPosition, _highlightPosition);
	auto endColumn = std::max(_cursorPosition, _highlightPosition);
	return _text.substr(startColumn, endColumn - startColumn);
}

void UI_Textbox::DeleteHighlightedText()
{
	auto startColumn = std::min(_cursorPosition, _highlightPosition);
	auto endColumn = std::max(_cursorPosition, _highlightPosition);
	_text.erase(startColumn, endColumn - startColumn);

	_cursorPosition = startColumn;
}

void UI_Textbox::InsertText(const std::string& text)
{
	_text.insert(_cursorPosition, text);
	//_cursorPosition += text.length();
}
