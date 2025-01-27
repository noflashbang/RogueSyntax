#include "UI.h"

UI::UI(Palette colors, uint32_t fontId, uint32_t fontsize)
{
	_menu.push_back(std::make_pair(MenuId{ "ID_FILE_MAIN", "File" }, std::vector<MenuId>{{"ID_NEW", "New"}, { "ID_OPEN","Open" }, { "ID_SAVE","Save" }, { "ID_SAVEAS","Save As" }, { "ID_EXIT","Exit" }}));
	_menu.push_back(std::make_pair(MenuId{ "ID_EDIT_MAIN", "Edit" }, std::vector<MenuId>{{"ID_UNDO", "Undo"}, { "ID_REDO","Redo" }, { "ID_CUT","Cut" }, { "ID_COPY","Copy" }, { "ID_PASTE","Paste" }}));
	_menu.push_back(std::make_pair(MenuId{ "ID_VIEW_MAIN", "View" }, std::vector<MenuId>{{"ID_ZOOMIN", "Zoom In"}, { "ID_ZOOMOUT","Zoom Out" }, { "ID_FULLSCREEN","Full Screen" }}));
	_menu.push_back(std::make_pair(MenuId{ "ID_HELP_MAIN", "Help" }, std::vector<MenuId>{{"ID_ABOUT", "About"}, { "ID_HELP","Help" }}));

	_details = "Details";
	_output = "Output";
	_editor = "Editor";
	_info = "Info";

	_colors = colors;
	_fontSize = fontsize;
	_fontId = fontId;
}

UI::~UI()
{
}

void UI::DoLayout()
{
	if (IsMouseButtonDown(0))
	{
		_menuIdActive = "";
	}
	SetMouseCursor(MOUSE_CURSOR_DEFAULT);

	CreateRoot();
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
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(30)}, .childGap = 16, .layoutDirection = CLAY_LEFT_TO_RIGHT }),
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
		CLAY_LAYOUT({ .padding = {16,16,8,8} }),
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
		CLAY_LAYOUT({ .sizing = { .width = CLAY_SIZING_GROW(0) }, .padding = {16,16,8,8} }),
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
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(30)} }),
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
	auto strContent = Clay_StringFromStdString(_editor);
	CLAY(
		CLAY_ID("Editor"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(_editorHeight)} }),
		CLAY_RECTANGLE({ .color = _colors.accentText })
	)
	{
		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = _colors.text, .fontId = _fontId, .fontSize = _fontSize}));
	}
}

void UI::CreateOutput()
{
	auto strContent = Clay_StringFromStdString(_output);
	CLAY(
		CLAY_ID("Output"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)} }),
		CLAY_RECTANGLE({ .color = _colors.background })
	)
	{
		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = _colors.text, .fontId = _fontId, .fontSize = _fontSize }));
	}
}

void UI::CreateInfo()
{
	auto strContent = Clay_StringFromStdString(_info);
	CLAY(
		CLAY_ID("InfoWindow"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(200), .height = CLAY_SIZING_GROW(0)} }),
		CLAY_RECTANGLE({ .color = _colors.background })
	)
	{
		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = _colors.text, .fontId = _fontId, .fontSize = _fontSize }));
	}
}

void UI::CreateDetails()
{
	auto strContent = Clay_StringFromStdString(_details);
	CLAY(
		CLAY_ID("DetailBar"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(20)} }),
		CLAY_RECTANGLE({ .color = _colors.accent })
	)
	{
		CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = _colors.text, .fontId = _fontId, .fontSize = _fontSize }));
	}
}

