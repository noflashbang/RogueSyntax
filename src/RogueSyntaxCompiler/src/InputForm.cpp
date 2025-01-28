#include "InputForm.h"

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


SimpleLineNumbering::SimpleLineNumbering(UIConfig config) : _config(config)
{
	for (size_t i = 0; i < 99; i++)
	{
		_lineNumbers.push_back(std::format("{:0>2}", i));
	}
}

void SimpleLineNumbering::LayoutLineNumbering(size_t line_number)
{
	CLAY(
		CLAY_IDI_LOCAL("LINENUM", line_number),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)(_config.fontSize * 2)), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		if (_lineNumbers.size() > line_number)
		{
			auto strContent = Clay_StringFromStdString(_lineNumbers.at(line_number));
			CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = _config.colors.text, .fontId = _config.fontId, .fontSize = _config.fontSize }));
		}
	}
}

BarCursorStrategy::BarCursorStrategy(UIConfig config) : _config(config)
{
}

void BarCursorStrategy::LayoutCursor()
{
	if (_cursorBlinker)
	{
		CLAY(
			CLAY_ID("CURSOR"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(1), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
			CLAY_FLOATING({ .attachment = {.element = CLAY_ATTACH_POINT_LEFT_CENTER, .parent = CLAY_ATTACH_POINT_LEFT_CENTER } }),
			CLAY_RECTANGLE({ .color = _config.colors.highlight })
		)
		{
		}
	}
	else
	{
		CLAY(
			CLAY_ID("CURSOR"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(1), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
			CLAY_FLOATING({ .attachment = {.element = CLAY_ATTACH_POINT_LEFT_CENTER, .parent = CLAY_ATTACH_POINT_LEFT_CENTER } }),
			CLAY_RECTANGLE({  })
		)
		{
		}
	}
}

HighlightCursorStrategy::HighlightCursorStrategy(UIConfig config) : _config(config)
{
}

void HighlightCursorStrategy::LayoutCursor()
{
	if (_cursorBlinker)
	{
		Clay_Color cursorColor = _config.colors.highlight;
		cursorColor.a = 128;

		CLAY(
			CLAY_ID("CURSOR"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)(_config.fontSize / 2)), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
			CLAY_FLOATING({ .attachment = {.element = CLAY_ATTACH_POINT_LEFT_CENTER, .parent = CLAY_ATTACH_POINT_LEFT_CENTER } }),
			CLAY_RECTANGLE({ .color = cursorColor })
		)
		{
		}
	}
	else
	{
		CLAY(
			CLAY_ID("CURSOR"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(1), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
			CLAY_FLOATING({ .attachment = {.element = CLAY_ATTACH_POINT_LEFT_CENTER, .parent = CLAY_ATTACH_POINT_LEFT_CENTER } }),
			CLAY_RECTANGLE({  })
		)
		{
		}
	}
}

InputForm::InputForm(const std::string& name, UIConfig config) : _config(config)
{
	_name = name;
	_cursorPosition = { 0, 0 };
	_highlightPosition = { 0, 0 };
	_hoverPosition = { 0, 0 };
	_highlighting = false;
	_clipBoardText = "";
}

InputForm::~InputForm()
{
	if (_lineNumberingStrategy != nullptr)
	{
		delete _lineNumberingStrategy;
		_lineNumberingStrategy = nullptr;
	}

	if (_cursorStrategy != nullptr)
	{
		delete _cursorStrategy;
		_cursorStrategy = nullptr;
	}
}

void InputForm::SetContent(const std::string& content)
{
	_inputFormLines = GetLinesBySplitor(content, '\n');
	_cursorPosition = { 0, 0 };
	_highlightPosition = { 0, 0 };
	_hoverPosition = { 0, 0 };
	_highlighting = false;
	_clipBoardText = "";
}

void InputForm::ProcessInputCommands(const std::vector<InputCmd>& cmds)
{
	for (auto& cmd : cmds)
	{
		switch (cmd.type)
		{
		case INPUT_CURSOR_LEFT:
		{
			_highlighting = cmd.flags == FLAG_HIGHLIGHT;
			if (_cursorPosition.column > 0)
			{
				_cursorPosition.column--;
			}
			break;
		}
		case INPUT_CURSOR_RIGHT:
		{
			_highlighting = cmd.flags == FLAG_HIGHLIGHT;
			if (_cursorPosition.column < _inputFormLines.at(_cursorPosition.line).length())
			{
				_cursorPosition.column++;
			}
			break;
		}
		case INPUT_CURSOR_UP:
		{
			_highlighting = cmd.flags == FLAG_HIGHLIGHT;
			if (_cursorPosition.line > 0)
			{
				_cursorPosition.line--;
			}
			break;
		}
		case INPUT_CURSOR_DOWN:
		{
			_highlighting = cmd.flags == FLAG_HIGHLIGHT;
			if (_cursorPosition.line < _inputFormLines.size() - 1)
			{
				_cursorPosition.line++;
			}
			break;
		}
		case INPUT_CURSOR_MOVE:
		{
			_highlighting = cmd.flags == FLAG_HIGHLIGHT;

			_cursorPosition.column = _hoverPosition.column;
			_cursorPosition.line = _hoverPosition.line;

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
			_cursorPosition.column = _hoverPosition.column;
			_cursorPosition.line = _hoverPosition.line;
			break;
		}
		case INPUT_INSERT:
		{
			if (_inputFormLines.size() == 0)
			{
				_inputFormLines.push_back("");
			}
			if (_highlighting)
			{
				DeleteHighlightedText(_inputFormLines);
				_highlighting = false;
			}
			char character = (char)cmd.flags;
			_inputFormLines.at(_cursorPosition.line).insert(_cursorPosition.column, 1, character);
			_cursorPosition.column++;
			break;
		}
		case INPUT_DELETE:
		{
			if (_highlighting)
			{
				DeleteHighlightedText(_inputFormLines);
				_highlighting = false;
			}
			else
			{
				if (cmd.flags == FLAG_DELETE_FWD)
				{
					if (_cursorPosition.column < _inputFormLines.at(_cursorPosition.line).length())
					{
						_inputFormLines.at(_cursorPosition.line).erase(_cursorPosition.column, 1);
					}
					else
					{
						if (_cursorPosition.line < _inputFormLines.size() - 1)
						{
							auto line = _inputFormLines.at(_cursorPosition.line);
							auto nextLine = _inputFormLines.at(_cursorPosition.line + 1);
							_inputFormLines.at(_cursorPosition.line) += nextLine;
							_inputFormLines.erase(_inputFormLines.begin() + _cursorPosition.line + 1);
						}
					}
				}
				else if (cmd.flags == FLAG_DELETE_BWD)
				{
					if (_cursorPosition.column > 0)
					{
						_cursorPosition.column--;
						_inputFormLines.at(_cursorPosition.line).erase(_cursorPosition.column, 1);
					}
					else
					{
						if (_cursorPosition.line > 0)
						{
							auto line = _inputFormLines.at(_cursorPosition.line);
							auto prevLine = _inputFormLines.at(_cursorPosition.line - 1);
							_cursorPosition.column = prevLine.length();
							_inputFormLines.at(_cursorPosition.line - 1) += line;
							_inputFormLines.erase(_inputFormLines.begin() + _cursorPosition.line);
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
			_cursorPosition.column = 0;
			break;
		}
		case INPUT_CURSOR_END:
		{
			_highlighting = cmd.flags == FLAG_HIGHLIGHT;
			_cursorPosition.column = _inputFormLines.at(_cursorPosition.line).length();
			break;
		}
		case INPUT_CURSOR_PAGEUP:
		{
			_cursorPosition.line--;
			if (_cursorPosition.line < 0)
			{
				_cursorPosition.line = 0;
			}
			break;
		}
		case INPUT_CURSOR_PAGEDOWN:
		{
			_cursorPosition.line++;
			if (_cursorPosition.line >= _inputFormLines.size())
			{
				_cursorPosition.line = _inputFormLines.size() - 1;
			}
			break;
		}
		case INPUT_NEWLINE:
		{
			if (_highlighting)
			{
				DeleteHighlightedText(_inputFormLines);
				_highlighting = false;
			}
			auto line = _inputFormLines.at(_cursorPosition.line);
			auto newLine = line.substr(_cursorPosition.column);
			_inputFormLines.at(_cursorPosition.line) = line.substr(0, _cursorPosition.column);
			_inputFormLines.insert(_inputFormLines.begin() + _cursorPosition.line + 1, newLine);
			_cursorPosition.line++;
			_cursorPosition.column = 0;
			break;
		}
		case INPUT_CURSOR_COPY:
		{
			CopyHighlightedText(_inputFormLines);
			break;
		}
		case INPUT_CURSOR_CUT:
		{
			CopyHighlightedText(_inputFormLines);
			DeleteHighlightedText(_inputFormLines);
			_highlighting = false;
			break;
		}
		case INPUT_CURSOR_PASTE:
		{
			InsertClipboardText(_inputFormLines);
			_highlighting = false;
			break;
		}
		}
	}

	//normalize the cursor position
	if (_cursorPosition.column < 0)
	{
		_cursorPosition.column = 0;
	}
	if (_cursorPosition.line < 0)
	{
		_cursorPosition.line = 0;
	}
	if (_inputFormLines.size() > 0 && _cursorPosition.line >= _inputFormLines.size())
	{
		_cursorPosition.line = _inputFormLines.size() - 1;
	}
	if (_inputFormLines.size() > 0 && _cursorPosition.column > _inputFormLines.at(_cursorPosition.line).length())
	{
		_cursorPosition.column = _inputFormLines.at(_cursorPosition.line).length();
	}

	//update the last cursor position
	if (!_highlighting)
	{
		_highlightPosition.column = _cursorPosition.column;
		_highlightPosition.line = _cursorPosition.line;
	}
}

void InputForm::Layout(bool hasFocus)
{
	if (_cursorStrategy != nullptr)
	{
		_cursorStrategy->Update(GetFrameTime());
	}

	size_t line_number = 0;
	for (auto& line : _inputFormLines)
	{
		CreateLine(hasFocus, line_number, line);
		line_number++;
	}
}

void InputForm::LayoutLineNumbering(size_t line_number)
{
	if (_lineNumberingStrategy != nullptr)
	{
		_lineNumberingStrategy->LayoutLineNumbering(line_number);
	}
}

void InputForm::LayoutCursor(bool hasFocus, size_t line_number, size_t index)
{
	if (_cursorStrategy != nullptr)
	{
		if (hasFocus && (_cursorPosition.line == line_number) && (_cursorPosition.column == index))
		{
			_cursorStrategy->LayoutCursor();
		}
	}
}

void InputForm::CreateLine(bool hasFocus, size_t line_number, const std::string_view line)
{
	uint32_t borderSize = 0;
	if (_cursorPosition.line == line_number)
	{
		borderSize = 1;
	}
	CLAY(
		CLAY_IDI_LOCAL("LINE", line_number),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
		//CLAY_BORDER_OUTSIDE({ .width = borderSize, .color = _config.colors.highlight }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		LayoutLineNumbering(line_number);

		for (auto index = 0; index < line.length(); index++)
		{
			char* character = (char*)line.data() + index;
			if (character != nullptr && *character != '\0')
			{
				CreateChar(hasFocus, line_number, index, character);
			}
		}

		//for drawing the cursor at the end of the line - also handles clicks at the end of the line
		CreatePlaceHolderChar(hasFocus, line_number, line.length(), &_cursorPlaceholder);
	}
}

void InputForm::CreatePlaceHolderChar(bool hasFocus, size_t line_number, size_t index, const char* character)
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
		LayoutCursor(hasFocus, line_number, index);

		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = textColor, .fontId = _config.fontId, .fontSize = _config.fontSize }));
		if (Clay_Hovered())
		{
			_hoverPosition.line = line_number;
			_hoverPosition.column = index;
		}
	}
}

void InputForm::CreateChar(bool hasFocus, size_t line_number, size_t index, const char* character)
{
	auto strContent = Clay_String{ .length = 1, .chars = character };
	Clay_Color textColor = _config.colors.text;
	Clay_Color backgroundColor = _config.colors.background;

	if (_highlighting && hasFocus)
	{
		auto startLine = std::min(_cursorPosition.line, _highlightPosition.line);
		auto endLine = std::max(_cursorPosition.line, _highlightPosition.line);

		auto startColumn = std::min(_cursorPosition.column, _highlightPosition.column);
		auto endColumn = std::max(_cursorPosition.column, _highlightPosition.column);

		if (startLine <= line_number && line_number <= endLine)
		{
			if (startLine == endLine)
			{
				if (startColumn <= index && index < endColumn)
				{
					backgroundColor = _config.colors.foreground;
					textColor = _config.colors.accentText;
				}
			}
			else if (startLine == line_number && index >= startColumn)
			{
				backgroundColor = _config.colors.foreground;
				textColor = _config.colors.accentText;
			}
			else if (endLine == line_number && index < endColumn)
			{
				backgroundColor = _config.colors.foreground;
				textColor = _config.colors.accentText;
			}
			else if (startLine < line_number && line_number < endLine)
			{
				backgroundColor = _config.colors.foreground;
				textColor = _config.colors.accentText;
			}
		}
	}

	CLAY(
		CLAY_IDI_LOCAL("COL", index),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)(_config.fontSize / 2)), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
		CLAY_RECTANGLE({ .color = backgroundColor })
	)
	{
		LayoutCursor(hasFocus, line_number, index);

		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = textColor, .fontId = _config.fontId, .fontSize = _config.fontSize }));
		if (Clay_Hovered())
		{
			_hoverPosition.line = line_number;
			_hoverPosition.column = index;
		}
	}
}

void InputForm::DeleteHighlightedText(std::vector<std::string>& data)
{
	if (_highlighting)
	{
		auto startLine = std::min(_cursorPosition.line, _highlightPosition.line);
		auto endLine = std::max(_cursorPosition.line, _highlightPosition.line);
		auto startColumn = std::min(_cursorPosition.column, _highlightPosition.column);
		auto endColumn = std::max(_cursorPosition.column, _highlightPosition.column);
		std::string text;
		if (startLine == endLine)
		{
			data.at(startLine).erase(startColumn, endColumn - startColumn);
		}
		else
		{
			data.at(startLine).erase(startColumn);
			data.at(endLine).erase(0, endColumn);
			data.erase(data.begin() + startLine + 1, data.begin() + endLine);
		}
	}
}

void InputForm::CopyHighlightedText(std::vector<std::string>& data)
{
	if (_highlighting)
	{
		auto startLine = std::min(_cursorPosition.line, _highlightPosition.line);
		auto endLine = std::max(_cursorPosition.line, _highlightPosition.line);
		auto startColumn = std::min(_cursorPosition.column, _highlightPosition.column);
		auto endColumn = std::max(_cursorPosition.column, _highlightPosition.column);
		std::string text;
		if (startLine == endLine)
		{
			text = data.at(startLine).substr(startColumn, endColumn - startColumn);
		}
		else
		{
			text = data.at(startLine).substr(startColumn) + '\n';
			for (auto i = startLine + 1; i < endLine; i++)
			{
				text += data.at(i) + '\n';
			}
			text += data.at(endLine).substr(0, endColumn);
		}
		SetClipboardText(text.c_str());
	}
}

void InputForm::InsertClipboardText(std::vector<std::string>& data)
{
	if (_highlighting)
	{
		DeleteHighlightedText(data);
	}

	auto textToInsert = GetLinesBySplitor(_clipBoardText, '\n');

	if (textToInsert.size() > 0)
	{
		auto startLine = _cursorPosition.line;
		auto startColumn = _cursorPosition.column;
		auto firstLine = textToInsert.at(0);
		data.at(startLine).insert(startColumn, firstLine);
		_cursorPosition.column += firstLine.length();

		if (textToInsert.size() >= 2)
		{
			// [0][1][2][3] -> 4, skip 0 and 3 -> 1,2
			for (auto i = 1; i < textToInsert.size() - 1; i++)
			{
				data.insert(data.begin() + startLine + i, textToInsert.at(i));
			}
		}

		if (textToInsert.size() > 1)
		{
			//add last line
			//check to see if we need to insert a new line
			if (startLine + textToInsert.size() - 1 > data.size() - 1)
			{
				data.push_back(textToInsert.at(textToInsert.size() - 1));
			}
			else
			{
				data.at(startLine + textToInsert.size() - 1).insert(0, textToInsert.at(textToInsert.size() - 1));
			}

			_cursorPosition.line += textToInsert.size() - 1;
			_cursorPosition.column = textToInsert.at(textToInsert.size() - 1).length();
		}
	}
}

void InputForm::SetClipboardText(const std::string& text)
{
	_clipBoardText = text;
}


