#include "UI.h"

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

UI::UI(Palette colors, uint32_t fontId, uint32_t fontsize)
{
	_menu.push_back(std::make_pair(MenuId{ "ID_FILE_MAIN", "File" }, std::vector<MenuId>{{"ID_NEW", "New"}, { "ID_OPEN","Open" }, { "ID_SAVE","Save" }, { "ID_SAVEAS","Save As" }, { "ID_EXIT","Exit" }}));
	_menu.push_back(std::make_pair(MenuId{ "ID_EDIT_MAIN", "Edit" }, std::vector<MenuId>{{"ID_UNDO", "Undo"}, { "ID_REDO","Redo" }, { "ID_CUT","Cut" }, { "ID_COPY","Copy" }, { "ID_PASTE","Paste" }}));
	_menu.push_back(std::make_pair(MenuId{ "ID_VIEW_MAIN", "View" }, std::vector<MenuId>{{"ID_ZOOMIN", "Zoom In"}, { "ID_ZOOMOUT","Zoom Out" }, { "ID_FULLSCREEN","Full Screen" }}));
	_menu.push_back(std::make_pair(MenuId{ "ID_HELP_MAIN", "Help" }, std::vector<MenuId>{{"ID_ABOUT", "About"}, { "ID_HELP","Help" }}));

	_clipBoardText = "";
	_details = "";
	_output = ":>";
	_editor = "let five = 5;\n let ten = 10;\n let add = fn(x, y) { x + y; };\n let result = add(five, ten);\n ";
	_info = "";

	
	for (size_t i = 0; i < 99; i++)
	{
		_lineNumbers.push_back(std::format("{:0>2}", i));
	}

	_colors = colors;
	_fontSize = fontsize;
	_fontId = fontId;

	uint16_t half = _fontSize / 2;
	_padding = { _fontSize, _fontSize, half, half };

	_formFocus = "Editor";

	//update the string views
	_editorLines = GetLinesBySplitor(_editor, '\n');
	_outputLines = GetLinesBySplitor(_output, '\n');
	_infoLines = GetLinesBySplitor(_info, '\n');
}

UI::~UI()
{
}

void UI::DoLayout()
{
	CreateInputCommands();
	ProcessInputCommands();

	if (_highlighting)
	{
		auto startLine = std::min(_cursorLine, _highlightLine);
		auto endLine = std::max(_cursorLine, _highlightLine);

		auto startColumn = std::min(_cursorColumn, _highlightColumn);
		auto endColumn = std::max(_cursorColumn, _highlightColumn);

		_details = std::format("Ln:{} Col: {} - Ln:{} Col: {}", startLine, startColumn, endLine, endColumn);
	}
	else
	{
		_details = std::format("Ln:{:0>2} Col: {:0>2}", _cursorLine, _cursorColumn);
	}

	if (IsMouseButtonDown(0))
	{
		_menuIdActive = "";
	}
	SetMouseCursor(MOUSE_CURSOR_DEFAULT);

	//handle cursor blink
	_blinkAccumulatedTime += GetFrameTime();
	if (_blinkAccumulatedTime > 0.5)
	{
		_blinkAccumulatedTime = 0.0;
		_cursorBlink = !_cursorBlink;
	}

	CreateRoot();
}

void UI::CreateLine(const std::string& context, size_t line_number, const std::string_view line, bool lineNumbers)
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
				CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)(_fontSize*2)), .height = CLAY_SIZING_FIXED((float)_fontSize)} }),
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

void UI::CreateChar(const std::string& context, size_t line_number, size_t index, const char* character)
{
	auto strContent = Clay_String{ .length = 1, .chars = character };
	Clay_Color textColor = _colors.text;
	Clay_Color backgroundColor = _colors.background;

	if (_highlighting)
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
							CLAY_FLOATING({ .attachment = { .element = CLAY_ATTACH_POINT_LEFT_CENTER, .parent = CLAY_ATTACH_POINT_LEFT_CENTER } }),
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
	}
}

void UI::CreateRoot()
{
	CLAY(
		CLAY_ID("Root"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _colors.background })
	)
	{
		CreateMenu();
		CreateActionBar();
		CreateMainForm();
		CreateDetails();
	}
}

void UI::CreateMenu()
{
	CLAY(
		CLAY_ID("Menu"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_fontSize*2)}, .childGap = _fontSize, .layoutDirection = CLAY_LEFT_TO_RIGHT }),
		CLAY_RECTANGLE({ .color = _colors.accent })
	)
	{
		for (auto& menu : _menu)
		{
			CreateMenuButton(menu.first, menu.second);
		}
	}
}

void UI::CreateMenuButton(const MenuId& name, const std::vector<MenuId>& items)
{
	auto id = Clay_StringFromStdString(name.id);
	auto strContent = Clay_StringFromStdString(name.name);
	uint32_t borderSize = 0;
	bool hovered = Clay_PointerOver(Clay__HashString(id, 0, 0));
	if (hovered)
	{
		borderSize = 2;
	}
	
	Clay_Color color = _colors.background;
	Clay_Color textColor = _colors.text;
	if (_menuIdActive == name.id)
	{
		color = _colors.foreground;
		textColor = _colors.accentText;
	}

	CLAY(
		Clay__AttachId(Clay__HashString(id, 0, 0)),
		CLAY_LAYOUT({ .padding = _padding }),
		CLAY_BORDER_OUTSIDE({ .width = borderSize, .color = _colors.highlight }),
		CLAY_RECTANGLE({ .color = color, .cornerRadius = 5 })
	)
	{
		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = textColor, .fontId = _fontId, .fontSize = _fontSize }));

		if (Clay_Hovered())
		{
			if (IsMouseButtonDown(0))
			{
				_menuIdActive = name.id;
			}
		}
		
		if (_menuIdActive == name.id)
		{
			CreateMenuDropDown(items);
		}
	}
}

void UI::CreateMenuDropDown(const std::vector<MenuId>& items)
{
	CLAY(
		CLAY_ID("FileMenu"),
		CLAY_FLOATING({
			.attachment = {
				.parent = CLAY_ATTACH_POINT_LEFT_BOTTOM
			},
			}),
		CLAY_LAYOUT({
			.padding = {0, 0, 8, 8 }
			})
	) {
		CLAY(
			CLAY_LAYOUT({
				.sizing = {
						.width = CLAY_SIZING_FIXED(150)
				},
				.layoutDirection = CLAY_TOP_TO_BOTTOM,
				}),
			CLAY_BORDER_OUTSIDE({ .width = 2, .color = _colors.highlight }),
			CLAY_RECTANGLE({
				.color = { 40, 40, 40, 255 },
				.cornerRadius = CLAY_CORNER_RADIUS(8)
				})
		)
		{
			for (auto& item : items)
			{
				CreateMenuDropDownButton(item);
			}
		}
	}
}

void UI::CreateMenuDropDownButton(const MenuId& name)
{
	auto id = Clay_StringFromStdString(name.id);
	auto strContent = Clay_StringFromStdString(name.name);

	bool hovered = Clay_PointerOver(Clay__HashString(id, 0, 0));

	CLAY(
		Clay__AttachId(Clay__HashString(id, 0, 0)),
		CLAY_LAYOUT({ .sizing = { .width = CLAY_SIZING_GROW(0) }, .padding = _padding }),
		CLAY_RECTANGLE({ .color = hovered ?_colors.foreground : _colors.background, .cornerRadius = 5 })
	)
	{
		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = hovered ? _colors.accentText : _colors.text, .fontId = _fontId, .fontSize = _fontSize }));
	}
}

void UI::CreateActionBar()
{
	CLAY(
		CLAY_ID("ActionBar"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_fontSize)} }),
		CLAY_RECTANGLE({ .color = _colors.accent })
	)
	{
	}
}

void UI::CreateMainForm()
{
	CLAY(
		CLAY_ID("MainContent"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_LEFT_TO_RIGHT }),
		CLAY_RECTANGLE({ .color = _colors.background })
	)
	{
		CreateIDEForm();
		CreateMainFormSpliter();
		CreateInfo();
	}
}

void UI::CreateMainFormSpliter()
{
	bool hovered = Clay_PointerOver(Clay_GetElementId(CLAY_STRING("MainFormSpliter")));

	if (hovered)
	{
		SetMouseCursor(MOUSE_CURSOR_RESIZE_EW);
	}

	if (hovered && IsMouseButtonDown(0))
	{
		_infoSizing = true;
	}
	if (_infoSizing)
	{
		SetMouseCursor(MOUSE_CURSOR_RESIZE_EW);
		if (IsMouseButtonUp(0))
		{
			SetMouseCursor(MOUSE_CURSOR_DEFAULT);
			_infoSizing = false;
		}
		_infoWidth += GetMouseDelta().x;
	}

	CLAY(
		CLAY_ID("MainFormSpliter"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(4), .height = CLAY_SIZING_GROW(0)} }),
		CLAY_RECTANGLE({ .color = _colors.text })
	)
	{
	}
}

void UI::CreateIDEForm()
{
	CLAY(
		CLAY_ID("IDEForm"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(_infoWidth), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _colors.text })
	)
	{
		CreateEditor();
		CreateIDEFormSpliter();
		CreateOutput();
	}
}

void UI::CreateIDEFormSpliter()
{
	bool hovered = Clay_PointerOver(Clay_GetElementId(CLAY_STRING("IDEFormSpliter")));

	if (hovered)
	{
		SetMouseCursor(MOUSE_CURSOR_RESIZE_NS);
	}


	if (hovered && IsMouseButtonDown(0))
	{
		_editorSizing = true;
	}

	if (_editorSizing)
	{
		SetMouseCursor(MOUSE_CURSOR_RESIZE_NS);
		if (IsMouseButtonUp(0))
		{
			SetMouseCursor(MOUSE_CURSOR_DEFAULT);
			_editorSizing = false;
		}
		_editorHeight += GetMouseDelta().y;
	}

	CLAY(
		CLAY_ID("IDEFormSpliter"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(4)} }),
		CLAY_RECTANGLE({ .color = _colors.text })
	)
	{
	}
}

void UI::CreateEditor()
{
	CLAY(
		CLAY_ID("Editor"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(_editorHeight)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _colors.background })
	)
	{
		auto line_number = 0;
		for (auto& line : _editorLines)
		{
			CreateLine("Editor",line_number, line, true);
			line_number++;
		}
	}
}

void UI::CreateOutput()
{
	CLAY(
		CLAY_ID("Output"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _colors.background })
	)
	{
		auto line_number = 0;
		for (auto& line : _outputLines)
		{
			CreateLine("Output", line_number, line, false);
			line_number++;
		}
	}
}

void UI::CreateInfo()
{
	CLAY(
		CLAY_ID("InfoWindow"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(200), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _colors.background })
	)
	{
		auto line_number = 0;
		for (auto& line : _infoLines)
		{
			CreateLine("Info", line_number, line, false);
			line_number++;
		}
	}
}

void UI::CreateDetails()
{
	auto strContent = Clay_StringFromStdString(_details);
	CLAY(
		CLAY_ID("DetailBar"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_fontSize)} }),
		CLAY_RECTANGLE({ .color = _colors.accent })
	)
	{
		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = _colors.text, .fontId = _fontId, .fontSize = _fontSize }));
	}
}

void UI::CreateInputCommands()
{
	_inputCmds.clear();

	bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
	bool ctrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
	bool isCapsLock = IsKeyDown(KEY_CAPS_LOCK);

	if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP))
	{
		_inputCmds.push_back(InputCmd{ INPUT_CURSOR_UP, shiftDown ? FLAG_HIGHLIGHT : FLAG_NORMAL });
	}
	if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN))
	{
		_inputCmds.push_back(InputCmd{ INPUT_CURSOR_DOWN, shiftDown ? FLAG_HIGHLIGHT : FLAG_NORMAL });
	}
	if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT))
	{
		_inputCmds.push_back(InputCmd{ INPUT_CURSOR_LEFT, shiftDown ? FLAG_HIGHLIGHT : FLAG_NORMAL });
	}
	if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT))
	{
		_inputCmds.push_back(InputCmd{ INPUT_CURSOR_RIGHT, shiftDown ? FLAG_HIGHLIGHT : FLAG_NORMAL });
	}
	if (IsKeyPressed(KEY_HOME))
	{
		_inputCmds.push_back(InputCmd{ INPUT_CURSOR_HOME, shiftDown ? FLAG_HIGHLIGHT : FLAG_NORMAL });
	}
	if (IsKeyPressed(KEY_END))
	{
		_inputCmds.push_back(InputCmd{ INPUT_CURSOR_END, shiftDown ? FLAG_HIGHLIGHT : FLAG_NORMAL });
	}
	if (IsKeyPressed(KEY_PAGE_UP))
	{
		_inputCmds.push_back(InputCmd{ INPUT_CURSOR_PAGEUP, shiftDown ? FLAG_HIGHLIGHT : FLAG_NORMAL });
	}
	if (IsKeyPressed(KEY_PAGE_DOWN))
	{
		_inputCmds.push_back(InputCmd{ INPUT_CURSOR_PAGEDOWN, shiftDown ? FLAG_HIGHLIGHT : FLAG_NORMAL });
	}
	if (IsKeyPressed(KEY_C) && ctrlDown)
	{
		_inputCmds.push_back(InputCmd{ INPUT_CURSOR_COPY, FLAG_NORMAL });
	}
	if (IsKeyPressed(KEY_X) && ctrlDown)
	{
		_inputCmds.push_back(InputCmd{ INPUT_CURSOR_CUT, FLAG_NORMAL });
	}
	if (IsKeyPressed(KEY_V) && ctrlDown)
	{
		_inputCmds.push_back(InputCmd{ INPUT_CURSOR_PASTE, FLAG_NORMAL });
	}
	if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
	{
		_inputCmds.push_back(InputCmd{ INPUT_NEWLINE, FLAG_NORMAL });
	}
	if (IsKeyPressed(KEY_BACKSPACE))
	{
		_inputCmds.push_back(InputCmd{ INPUT_DELETE, FLAG_DELETE_BWD });
	}
	if (IsKeyPressed(KEY_DELETE))
	{
		_inputCmds.push_back(InputCmd{ INPUT_DELETE, FLAG_DELETE_FWD });
	}

	//handle text input
	int key = GetKeyPressed();
	while (key != 0 && !ctrlDown)
	{
		if (key == KEY_A)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'A' : 'a') });
		}

		if (key == KEY_B)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'B' : 'b') });
		}

		if (key == KEY_C)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'C' : 'c') });
		}

		if (key == KEY_D)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'D' : 'd') });
		}

		if (key == KEY_E)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'E' : 'e') });
		}

		if (key == KEY_F)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'F' : 'f') });
		}

		if (key == KEY_G)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'G' : 'g') });
		}

		if (key == KEY_H)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'H' : 'h') });
		}

		if (key == KEY_I)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'I' : 'i') });
		}

		if (key == KEY_J)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'J' : 'j') });
		}

		if (key == KEY_K)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'K' : 'k') });
		}

		if (key == KEY_L)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'L' : 'l') });
		}

		if (key == KEY_M)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'M' : 'm') });
		}

		if (key == KEY_N)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'N' : 'n') });
		}

		if (key == KEY_O)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'O' : 'o') });
		}

		if (key == KEY_P)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'P' : 'p') });
		}

		if (key == KEY_Q)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'Q' : 'q') });
		}

		if (key == KEY_R)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'R' : 'r') });
		}

		if (key == KEY_S)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'S' : 's') });
		}

		if (key == KEY_T)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'T' : 't') });
		}

		if (key == KEY_U)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'U' : 'u') });
		}

		if (key == KEY_V)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'V' : 'v') });
		}

		if (key == KEY_W)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'W' : 'w') });
		}

		if (key == KEY_X)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'X' : 'x') });
		}

		if (key == KEY_Y)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'Y' : 'y') });
		}

		if (key == KEY_Z)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown | isCapsLock ? 'Z' : 'z') });
		}

		if (key == KEY_APOSTROPHE)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '"' : '\'') });
		}

		if (key == KEY_COMMA)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '<' : ',') });
		}

		if (key == KEY_MINUS)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '_' : '-') });
		}

		if (key == KEY_PERIOD)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '>' : '.') });
		}

		if (key == KEY_SLASH)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '?' : '/') });
		}

		if (key == KEY_SEMICOLON)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? ':' : ';') });
		}

		if (key == KEY_EQUAL)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '+' : '=') });
		}

		if (key == KEY_LEFT_BRACKET)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '{' : '[') });
		}

		if (key == KEY_BACKSLASH)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '|' : '\\') });
		}

		if (key == KEY_RIGHT_BRACKET)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '}' : ']') });
		}

		if (key == KEY_GRAVE)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '~' : '`') });
		}

		if (key == KEY_SPACE)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)' ' });
		}

		if (key == KEY_TAB)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'\t' });
		}

		if (key == KEY_ZERO)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? ')' : '0') });
		}

		if (key == KEY_ONE)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '!' : '1') });
		}

		if (key == KEY_TWO)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '@' : '2') });
		}

		if (key == KEY_THREE)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '#' : '3') });
		}

		if (key == KEY_FOUR)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '$' : '4') });
		}

		if (key == KEY_FIVE)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '%' : '5') });
		}

		if (key == KEY_SIX)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '^' : '6') });
		}

		if (key == KEY_SEVEN)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '&' : '7') });
		}

		if (key == KEY_EIGHT)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '*' : '8') });
		}

		if (key == KEY_NINE)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(shiftDown ? '(' : '9') });
		}

		if (key == KEY_KP_0)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'0' });
		}

		if (key == KEY_KP_1)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'1' });
		}

		if (key == KEY_KP_2)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'2' });
		}

		if (key == KEY_KP_3)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'3' });
		}

		if (key == KEY_KP_4)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'4' });
		}

		if (key == KEY_KP_5)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'5' });
		}

		if (key == KEY_KP_6)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'6' });
		}

		if (key == KEY_KP_7)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'7' });
		}

		if (key == KEY_KP_8)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'8' });
		}

		if (key == KEY_KP_9)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'9' });
		}

		if (key == KEY_KP_DECIMAL)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'.' });
		}

		if (key == KEY_KP_DIVIDE)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'/' });
		}

		if (key == KEY_KP_MULTIPLY)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'*' });
		}

		if (key == KEY_KP_SUBTRACT)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'-' });
		}

		if (key == KEY_KP_ADD)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'+' });
		}

		if (key == KEY_KP_EQUAL)
		{
			_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)'=' });
		}
		key = GetKeyPressed();
	}
}
void UI::ProcessInputCommands()
{
	for (auto& cmd : _inputCmds)
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
				if (_cursorColumn < _editorLines.at(_cursorLine).length())
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
				if (_cursorLine < _editorLines.size() - 1)
				{
					_cursorLine++;
				}
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
				_editorLines.at(_cursorLine).insert(_cursorColumn, 1, character);
				_cursorColumn++;
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
						if (_cursorColumn < _editorLines.at(_cursorLine).length())
						{
							_editorLines.at(_cursorLine).erase(_cursorColumn, 1);
						}
						else
						{
							if (_cursorLine < _editorLines.size() - 1)
							{
								auto line = _editorLines.at(_cursorLine);
								auto nextLine = _editorLines.at(_cursorLine + 1);
								_editorLines.at(_cursorLine) += nextLine;
								_editorLines.erase(_editorLines.begin() + _cursorLine + 1);
							}
						}
					}
					else if (cmd.flags == FLAG_DELETE_BWD)
					{
						if (_cursorColumn > 0)
						{
							_cursorColumn--;
							_editorLines.at(_cursorLine).erase(_cursorColumn, 1);
						}
						else
						{
							if (_cursorLine > 0)
							{
								auto line = _editorLines.at(_cursorLine);
								auto prevLine = _editorLines.at(_cursorLine - 1);
								_cursorColumn = prevLine.length();
								_editorLines.at(_cursorLine - 1) += line;
								_editorLines.erase(_editorLines.begin() + _cursorLine);
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
				_cursorColumn = _editorLines.at(_cursorLine).length();
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
				if (_cursorLine >= _editorLines.size())
				{
					_cursorLine = _editorLines.size() - 1;
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
				auto line = _editorLines.at(_cursorLine);
				auto newLine = line.substr(_cursorColumn);
				_editorLines.at(_cursorLine) = line.substr(0, _cursorColumn);
				_editorLines.insert(_editorLines.begin() + _cursorLine + 1, newLine);
				_cursorLine++;
				_cursorColumn = 0;
				break;
			}
			case INPUT_CURSOR_COPY:
			{
				CopyHighlightedText();
				break;
			}
			case INPUT_CURSOR_CUT:
			{
				CopyHighlightedText();
				DeleteHighlightedText();
				_highlighting = false;
				break;
			}
			case INPUT_CURSOR_PASTE:
			{
				InsertClipboardText();
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
	if (_cursorLine >= _editorLines.size())
	{
		_cursorLine = _editorLines.size() - 1;
	}
	if (_cursorColumn > _editorLines.at(_cursorLine).length())
	{
		_cursorColumn = _editorLines.at(_cursorLine).length();
	}
	
	//update the last cursor position
	if (!_highlighting)
	{
		_highlightColumn = _cursorColumn;
		_highlightLine = _cursorLine;
	}
}

void UI::DeleteHighlightedText()
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
			_editorLines.at(startLine).erase(startColumn, endColumn - startColumn);
		}
		else
		{
			_editorLines.at(startLine).erase(startColumn);
			_editorLines.at(endLine).erase(0, endColumn);
			_editorLines.erase(_editorLines.begin() + startLine + 1, _editorLines.begin() + endLine);
		}
	}
}

void UI::CopyHighlightedText()
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
			text = _editorLines.at(startLine).substr(startColumn, endColumn - startColumn);
		}
		else
		{
			text = _editorLines.at(startLine).substr(startColumn) + '\n';
			for (auto i = startLine + 1; i < endLine; i++)
			{
				text += _editorLines.at(i) + '\n';
			}
			text += _editorLines.at(endLine).substr(0, endColumn);
		}
		SetClipboardText(text.c_str());
	}
}

void UI::InsertClipboardText()
{
	if (_highlighting)
	{
		DeleteHighlightedText();
	}

	auto textToInsert = GetLinesBySplitor(_clipBoardText, '\n');

	if (textToInsert.size() > 0)
	{
		auto startLine = _cursorLine;
		auto startColumn = _cursorColumn;
		auto firstLine = textToInsert.at(0);
		_editorLines.at(startLine).insert(startColumn, firstLine);
		_cursorColumn += firstLine.length();

		if (textToInsert.size() >= 2)
		{
			// [0][1][2][3] -> 4, skip 0 and 3 -> 1,2
			for (auto i = 1; i < textToInsert.size() - 1; i++)
			{
				_editorLines.insert(_editorLines.begin() + startLine + i, textToInsert.at(i));
			}
		}
		
		if (textToInsert.size() > 1)
		{
			//add last line
			//check to see if we need to insert a new line
			if (startLine + textToInsert.size() - 1 > _editorLines.size() - 1)
			{
				_editorLines.push_back(textToInsert.at(textToInsert.size() - 1));
			}
			else
			{
				_editorLines.at(startLine + textToInsert.size() - 1).insert(0, textToInsert.at(textToInsert.size() - 1));
			}

			_cursorLine += textToInsert.size() - 1;
			_cursorColumn = textToInsert.at(textToInsert.size() - 1).length();
		}
	}
}

void UI::SetClipboardText(const std::string& text)
{
	_clipBoardText = text;
}
