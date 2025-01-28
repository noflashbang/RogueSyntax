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

InputForm::InputForm(const std::string& name)
{
	_name = name;

	for (size_t i = 0; i < 99; i++)
	{
		_lineNumbers.push_back(std::format("{:0>2}", i));
	}
}

InputForm::~InputForm()
{
}

void InputForm::SetContent(const std::string& content)
{
	_inputFormLines = GetLinesBySplitor(content, '\n');
	_cursorLine = 0;
	_cursorColumn = 0;
	_highlighting = false;
	_highlightLine = 0;
	_highlightColumn = 0;
	_hoverLine = 0;
	_hoverColumn = 0;
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
			if (_cursorColumn > 0)
			{
				_cursorColumn--;
			}
			break;
		}
		case INPUT_CURSOR_RIGHT:
		{
			_highlighting = cmd.flags == FLAG_HIGHLIGHT;
			if (_cursorColumn < _inputFormLines.at(_cursorLine).length())
			{
				_cursorColumn++;
			}
			break;
		}
		case INPUT_CURSOR_UP:
		{
			_highlighting = cmd.flags == FLAG_HIGHLIGHT;
			if (_cursorLine > 0)
			{
				_cursorLine--;
			}
			break;
		}
		case INPUT_CURSOR_DOWN:
		{
			_highlighting = cmd.flags == FLAG_HIGHLIGHT;
			if (_cursorLine < _inputFormLines.size() - 1)
			{
				_cursorLine++;
			}
			break;
		}
		case INPUT_CURSOR_MOVE:
		{
			_highlighting = cmd.flags == FLAG_HIGHLIGHT;

			_cursorColumn = _hoverColumn;
			_cursorLine = _hoverLine;

			if (!_highlighting)
			{
				_highlightColumn = _cursorColumn;
				_highlightLine = _cursorLine;
			}

			break;
		}
		case INPUT_CURSOR_DRAG:
		{
			if (!_highlighting)
			{
				_highlightColumn = _cursorColumn;
				_highlightLine = _cursorLine;
			}
			_highlighting = true;
			_cursorColumn = _hoverColumn;
			_cursorLine = _hoverLine;
			break;
		}
		case INPUT_INSERT:
		{
			if (_highlighting)
			{
				DeleteHighlightedText(_inputFormLines);
				_highlighting = false;
			}
			char character = (char)cmd.flags;
			_inputFormLines.at(_cursorLine).insert(_cursorColumn, 1, character);
			_cursorColumn++;
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
					if (_cursorColumn < _inputFormLines.at(_cursorLine).length())
					{
						_inputFormLines.at(_cursorLine).erase(_cursorColumn, 1);
					}
					else
					{
						if (_cursorLine < _inputFormLines.size() - 1)
						{
							auto line = _inputFormLines.at(_cursorLine);
							auto nextLine = _inputFormLines.at(_cursorLine + 1);
							_inputFormLines.at(_cursorLine) += nextLine;
							_inputFormLines.erase(_inputFormLines.begin() + _cursorLine + 1);
						}
					}
				}
				else if (cmd.flags == FLAG_DELETE_BWD)
				{
					if (_cursorColumn > 0)
					{
						_cursorColumn--;
						_inputFormLines.at(_cursorLine).erase(_cursorColumn, 1);
					}
					else
					{
						if (_cursorLine > 0)
						{
							auto line = _inputFormLines.at(_cursorLine);
							auto prevLine = _inputFormLines.at(_cursorLine - 1);
							_cursorColumn = prevLine.length();
							_inputFormLines.at(_cursorLine - 1) += line;
							_inputFormLines.erase(_inputFormLines.begin() + _cursorLine);
							_cursorLine--;
						}
					}
				}
			}
			break;
		}
		case INPUT_CURSOR_HOME:
		{
			_highlighting = cmd.flags == FLAG_HIGHLIGHT;
			_cursorColumn = 0;
			break;
		}
		case INPUT_CURSOR_END:
		{
			_highlighting = cmd.flags == FLAG_HIGHLIGHT;
			_cursorColumn = _inputFormLines.at(_cursorLine).length();
			break;
		}
		case INPUT_CURSOR_PAGEUP:
		{
			_cursorLine--;
			if (_cursorLine < 0)
			{
				_cursorLine = 0;
			}
			break;
		}
		case INPUT_CURSOR_PAGEDOWN:
		{
			_cursorLine++;
			if (_cursorLine >= _inputFormLines.size())
			{
				_cursorLine = _inputFormLines.size() - 1;
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
			auto line = _inputFormLines.at(_cursorLine);
			auto newLine = line.substr(_cursorColumn);
			_inputFormLines.at(_cursorLine) = line.substr(0, _cursorColumn);
			_inputFormLines.insert(_inputFormLines.begin() + _cursorLine + 1, newLine);
			_cursorLine++;
			_cursorColumn = 0;
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
	if (_cursorColumn < 0)
	{
		_cursorColumn = 0;
	}
	if (_cursorLine < 0)
	{
		_cursorLine = 0;
	}
	if (_cursorLine >= _inputFormLines.size())
	{
		_cursorLine = _inputFormLines.size() - 1;
	}
	if (_cursorColumn > _inputFormLines.at(_cursorLine).length())
	{
		_cursorColumn = _inputFormLines.at(_cursorLine).length();
	}

	//update the last cursor position
	if (!_highlighting)
	{
		_highlightColumn = _cursorColumn;
		_highlightLine = _cursorLine;
	}
}

void InputForm::Layout()
{

}

void InputForm::CreateLine(const std::string& context, size_t line_number, const std::string_view line, bool lineNumbers)
{
	CLAY(
		CLAY_IDI_LOCAL("LINE", line_number),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_fontSize)} }),
		CLAY_RECTANGLE({ .color = _colors.background })
	)
	{
		if (lineNumbers)
		{
			CLAY(
				CLAY_IDI_LOCAL("LINENUM", line_number),
				CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)(_fontSize * 2)), .height = CLAY_SIZING_FIXED((float)_fontSize)} }),
				CLAY_RECTANGLE({ .color = _colors.background })
			)
			{
				if (_lineNumbers.size() > line_number)
				{
					auto strContent = Clay_StringFromStdString(_lineNumbers.at(line_number));
					CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = _colors.text, .fontId = _fontId, .fontSize = _fontSize }));
				}
			}
		}
		for (auto index = 0; index < line.length(); index++)
		{
			char* character = (char*)line.data() + index;
			if (character != nullptr && *character != '\0')
			{
				CreateChar(context, line_number, index, character);
			}
		}
		//draw a cursor at the end of the line
		if (_cursorLine == line_number)
		{
			if (_cursorColumn == line.length())
			{
				CreateChar(context, line_number, line.length(), &_cursorPlaceholder);
			}
		}
	}
}

void InputForm::CreateChar(const std::string& context, size_t line_number, size_t index, const char* character)
{
	auto strContent = Clay_String{ .length = 1, .chars = character };
	Clay_Color textColor = _colors.text;
	Clay_Color backgroundColor = _colors.background;

	if (_highlighting && context == _formFocus)
	{
		auto startLine = std::min(_cursorLine, _highlightLine);
		auto endLine = std::max(_cursorLine, _highlightLine);

		auto startColumn = std::min(_cursorColumn, _highlightColumn);
		auto endColumn = std::max(_cursorColumn, _highlightColumn);

		if (startLine <= line_number && line_number <= endLine)
		{
			if (startLine == endLine)
			{
				if (startColumn <= index && index < endColumn)
				{
					backgroundColor = _colors.foreground;
					textColor = _colors.accentText;
				}
			}
			else if (startLine == line_number && index >= startColumn)
			{
				backgroundColor = _colors.foreground;
				textColor = _colors.accentText;
			}
			else if (endLine == line_number && index < endColumn)
			{
				backgroundColor = _colors.foreground;
				textColor = _colors.accentText;
			}
			else if (startLine < line_number && line_number < endLine)
			{
				backgroundColor = _colors.foreground;
				textColor = _colors.accentText;
			}
		}
	}

	CLAY(
		CLAY_IDI_LOCAL("COL", index),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)(_fontSize / 2)), .height = CLAY_SIZING_FIXED((float)_fontSize)} }),
		CLAY_RECTANGLE({ .color = backgroundColor })
	)
	{
		bool hasCursor = false;
		if (_formFocus == context)
		{
			if (_cursorLine == line_number)
			{
				if (_cursorColumn == index)
				{
					hasCursor = true;
					if (_cursorBlink)
					{
						CLAY(
							CLAY_ID("CURSOR"),
							CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(1), .height = CLAY_SIZING_FIXED((float)_fontSize)} }),
							CLAY_FLOATING({ .attachment = {.element = CLAY_ATTACH_POINT_LEFT_CENTER, .parent = CLAY_ATTACH_POINT_LEFT_CENTER } }),
							CLAY_RECTANGLE({ .color = _colors.highlight })
						)
						{
						}
					}
					else
					{
						CLAY(
							CLAY_ID("CURSOR"),
							CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(1), .height = CLAY_SIZING_FIXED((float)_fontSize)} }),
							CLAY_FLOATING({ .attachment = {.element = CLAY_ATTACH_POINT_LEFT_CENTER, .parent = CLAY_ATTACH_POINT_LEFT_CENTER } }),
							CLAY_RECTANGLE({  })
						)
						{
						}
					}
				}
			}
		}
		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = textColor, .fontId = _fontId, .fontSize = _fontSize }));
		if (Clay_Hovered())
		{
			_hoverLine = line_number;
			_hoverColumn = index;
		}
	}
}

void InputForm::DeleteHighlightedText(std::vector<std::string>& data)
{
	if (_highlighting)
	{
		auto startLine = std::min(_cursorLine, _highlightLine);
		auto endLine = std::max(_cursorLine, _highlightLine);
		auto startColumn = std::min(_cursorColumn, _highlightColumn);
		auto endColumn = std::max(_cursorColumn, _highlightColumn);
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
		auto startLine = std::min(_cursorLine, _highlightLine);
		auto endLine = std::max(_cursorLine, _highlightLine);
		auto startColumn = std::min(_cursorColumn, _highlightColumn);
		auto endColumn = std::max(_cursorColumn, _highlightColumn);
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
		auto startLine = _cursorLine;
		auto startColumn = _cursorColumn;
		auto firstLine = textToInsert.at(0);
		data.at(startLine).insert(startColumn, firstLine);
		_cursorColumn += firstLine.length();

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

			_cursorLine += textToInsert.size() - 1;
			_cursorColumn = textToInsert.at(textToInsert.size() - 1).length();
		}
	}
}

void InputForm::SetClipboardText(const std::string& text)
{
	_clipBoardText = text;
}