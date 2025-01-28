#include "UI.h"



UI::UI(UIConfig config) : _editorForm("Editor", config), _outputForm("Output",config), _infoForm("Info", config),  _config(config)
{
	_menu.push_back(std::make_pair(MenuId{ "ID_FILE_MAIN", "File" }, std::vector<MenuId>{{"ID_NEW", "New"}, { "ID_OPEN","Open" }, { "ID_SAVE","Save" }, { "ID_SAVEAS","Save As" }, { "ID_EXIT","Exit" }}));
	_menu.push_back(std::make_pair(MenuId{ "ID_EDIT_MAIN", "Edit" }, std::vector<MenuId>{{"ID_UNDO", "Undo"}, { "ID_REDO","Redo" }, { "ID_CUT","Cut" }, { "ID_COPY","Copy" }, { "ID_PASTE","Paste" }}));
	_menu.push_back(std::make_pair(MenuId{ "ID_VIEW_MAIN", "View" }, std::vector<MenuId>{{"ID_ZOOMIN", "Zoom In"}, { "ID_ZOOMOUT","Zoom Out" }, { "ID_FULLSCREEN","Full Screen" }}));
	_menu.push_back(std::make_pair(MenuId{ "ID_HELP_MAIN", "Help" }, std::vector<MenuId>{{"ID_ABOUT", "About"}, { "ID_HELP","Help" }}));

	_details = "";
	_output = ":>";
	_editor = "let five = 5;\n let ten = 10;\n let add = fn(x, y) { x + y; };\n let result = add(five, ten);\n ";
	_info = "";

	_formFocus = "Editor";
}

UI::~UI()
{
}

void UI::DoLayout()
{
	CreateInputCommands();

	if (_formFocus == "Editor")
	{
		_editorForm.ProcessInputCommands(_inputCmds);
	}
	else if (_formFocus == "Output")
	{
		
	}
	else if (_formFocus == "Info")
	{
		
	}

	//if (_highlighting)
	//{
	//	auto startLine = std::min(_cursorLine, _highlightLine);
	//	auto endLine = std::max(_cursorLine, _highlightLine);
	//
	//	auto startColumn = std::min(_cursorColumn, _highlightColumn);
	//	auto endColumn = std::max(_cursorColumn, _highlightColumn);
	//
	//	_details = std::format("Ln:{:0>2} Col: {:0>2} - Ln:{:0>2} Col: {:0>2} | Mouse Ln: {:0>2} Col: {:0>2}", startLine, startColumn, endLine, endColumn, _hoverLine, _hoverColumn);
	//}
	//else
	//{
	//	_details = std::format("Ln:{:0>2} Col: {:0>2} | Mouse Ln: {:0>2} Col: {:0>2}", _cursorLine, _cursorColumn, _hoverLine, _hoverColumn);
	//}

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



void UI::CreateRoot()
{
	CLAY(
		CLAY_ID("Root"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
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
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize*2)}, .childGap = _config.fontSize, .layoutDirection = CLAY_LEFT_TO_RIGHT }),
		CLAY_RECTANGLE({ .color = _config.colors.accent })
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
	
	Clay_Color color = _config.colors.background;
	Clay_Color textColor = _config.colors.text;
	if (_menuIdActive == name.id)
	{
		color = _config.colors.foreground;
		textColor = _config.colors.accentText;
	}

	CLAY(
		Clay__AttachId(Clay__HashString(id, 0, 0)),
		CLAY_LAYOUT({ .padding = _config.padding }),
		CLAY_BORDER_OUTSIDE({ .width = borderSize, .color = _config.colors.highlight }),
		CLAY_RECTANGLE({ .color = color, .cornerRadius = 5 })
	)
	{
		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = textColor, .fontId = _config.fontId, .fontSize = _config.fontSize }));

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
			CLAY_BORDER_OUTSIDE({ .width = 2, .color = _config.colors.highlight }),
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
		CLAY_LAYOUT({ .sizing = { .width = CLAY_SIZING_GROW(0) }, .padding = _config.padding }),
		CLAY_RECTANGLE({ .color = hovered ? _config.colors.foreground : _config.colors.background, .cornerRadius = 5 })
	)
	{
		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = hovered ? _config.colors.accentText : _config.colors.text, .fontId = _config.fontId, .fontSize = _config.fontSize }));
	}
}

void UI::CreateActionBar()
{
	CLAY(
		CLAY_ID("ActionBar"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
		CLAY_RECTANGLE({ .color = _config.colors.accent })
	)
	{
	}
}

void UI::CreateMainForm()
{
	CLAY(
		CLAY_ID("MainContent"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_LEFT_TO_RIGHT }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
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
		CLAY_RECTANGLE({ .color = _config.colors.text })
	)
	{
	}
}

void UI::CreateIDEForm()
{
	CLAY(
		CLAY_ID("IDEForm"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(_infoWidth), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _config.colors.text })
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
		CLAY_RECTANGLE({ .color = _config.colors.text })
	)
	{
	}
}

void UI::CreateEditor()
{
	CLAY(
		CLAY_ID("Editor"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(_editorHeight)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		if (Clay_Hovered() && IsMouseButtonDown(0))
		{
			_formFocus = "Editor";
		}

		_editorForm.Layout();
	}
}

void UI::CreateOutput()
{
	CLAY(
		CLAY_ID("Output"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		if (Clay_Hovered() && IsMouseButtonDown(0))
		{
			_formFocus = "Output";
		}

		_outputForm.Layout();
	}
}

void UI::CreateInfo()
{
	CLAY(
		CLAY_ID("InfoWindow"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(200), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		if (Clay_Hovered() && IsMouseButtonDown(0))
		{
			_formFocus = "Info";
		}

		_infoForm.Layout();
	}
}

void UI::CreateDetails()
{
	auto strContent = Clay_StringFromStdString(_details);
	CLAY(
		CLAY_ID("DetailBar"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
		CLAY_RECTANGLE({ .color = _config.colors.accent })
	)
	{
		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = _config.colors.text, .fontId = _config.fontId, .fontSize = _config.fontSize }));
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
	
	if (IsMouseButtonDown(0))
	{
		_mouseBtnOneDownTime += GetFrameTime();
	}
	if (IsMouseButtonReleased(0))
	{
		_mouseBtnOneDownTime = 0;
	}
	if (IsMouseButtonPressed(0) || IsMouseButtonDown(0))
	{
		if (_mouseBtnOneDownTime > 0.05)
		{
			_inputCmds.push_back(InputCmd{ INPUT_CURSOR_DRAG, FLAG_DRAG });
		}
		else
		{
			_inputCmds.push_back(InputCmd{ INPUT_CURSOR_MOVE, shiftDown ? FLAG_HIGHLIGHT : FLAG_NORMAL });
		}
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

	if (IsKeyPressedRepeat(KEY_BACKSPACE))
	{
		_inputCmds.push_back(InputCmd{ INPUT_DELETE, FLAG_DELETE_BWD });
	}
	if (IsKeyPressedRepeat(KEY_DELETE))
	{
		_inputCmds.push_back(InputCmd{ INPUT_DELETE, FLAG_DELETE_FWD });
	}
	if (IsKeyPressedRepeat(KEY_ENTER) || IsKeyPressedRepeat(KEY_KP_ENTER))
	{
		_inputCmds.push_back(InputCmd{ INPUT_NEWLINE, FLAG_NORMAL });
	}
	if (IsKeyPressedRepeat(KEY_SPACE))
	{
		_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)' ' });
	}

	const std::vector<uint32_t> keyList{ KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_APOSTROPHE, KEY_COMMA, KEY_MINUS, KEY_PERIOD, KEY_SLASH, KEY_SEMICOLON, KEY_EQUAL, KEY_LEFT_BRACKET, KEY_BACKSLASH, KEY_RIGHT_BRACKET, KEY_GRAVE, KEY_SPACE, KEY_TAB, KEY_ZERO, KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT, KEY_NINE, KEY_KP_0, KEY_KP_1, KEY_KP_2, KEY_KP_3, KEY_KP_4, KEY_KP_5, KEY_KP_6, KEY_KP_7, KEY_KP_8, KEY_KP_9, KEY_KP_DECIMAL, KEY_KP_DIVIDE, KEY_KP_MULTIPLY, KEY_KP_SUBTRACT, KEY_KP_ADD, KEY_KP_EQUAL };
	for (auto& key : keyList)
	{
		if (IsKeyPressedRepeat(key))
		{
			if (('A' <= key && key <= 'Z'))
			{
				if (shiftDown || isCapsLock)
				{
					_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)key });
				}
				else
				{
					_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)(key+32) });
				}
			}
			else
			{
				_inputCmds.push_back(InputCmd{ INPUT_INSERT, (uint32_t)key });
			}
		}
	}
}

