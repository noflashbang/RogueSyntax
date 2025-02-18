#include "UI_Layout_SaveForm.h"


UI_Layout_SaveForm::UI_Layout_SaveForm(uint16_t width, uint16_t height, const std::string& name, UIConfig config, std::shared_ptr<UIEventObserver<std::string>> focusChanged) : _config(config), _eventCurrentFocusObserver(focusChanged)
{
	_eventCurrentFocus = UIEventAgent<std::string>();

	_layoutDimensions.width = width;
	_layoutDimensions.height = height;
	_name = name;
	_title = "Save File";
	_path = "";

	_closeBtn = std::make_unique<UI_Button>(config, "closeButton");
	_closeBtn->SetLayoutDimensions(LayoutDimensions{ (float)_config.fontSize, (float)_config.fontSize });
	_closeBtn->SetText("X");
	_closeBtnConnection = _closeBtn->onClick() += [this]() { this->Close(); };

	_pathTxt = std::make_unique<UI_Textbox>(config, "pathTxt", _eventCurrentFocus.Subscribe(), std::make_unique<BarCursorStrategy>(_config));
	_pathTxt->SetLayoutDimensions(LayoutDimensions((float)width - 32, (float)_config.fontSize));

	_dirTxt = std::make_unique<UI_Textbox>(config, "dirTxt", _eventCurrentFocus.Subscribe(), std::make_unique<BarCursorStrategy>(_config));
	_dirTxt->SetLayoutDimensions(LayoutDimensions((float)width - 32, (float)_config.fontSize));

	_filesArea = std::make_unique<UI_Listbox>(config, "filesArea", _eventCurrentFocus.Subscribe(), std::vector<std::string>());
	_filesArea->SetLayoutDimensions(LayoutDimensions((float)width - 32, (float)_config.fontSize * 12));

	_saveBtn = std::make_unique<UI_Button>(config, "saveButton");
	_saveBtn->SetLayoutDimensions(LayoutDimensions{ (float)_config.fontSize * 5, (float)_config.fontSize });
	_saveBtn->SetText("Save");
	_saveBtnConnection = _saveBtn->onClick() += [this]() { this->Save(); };

	_dir = std::filesystem::current_path().string() + "\\" + "example";
	SetFileExplorer();
}

UI_Layout_SaveForm::~UI_Layout_SaveForm()
{
}

void UI_Layout_SaveForm::ProcessInputCommand(const InputCmd& cmd)
{
	if (HasFocus())
	{
		_pathTxt->ProcessInputCommand(cmd);
		_filesArea->ProcessInputCommand(cmd);
	}
}

void UI_Layout_SaveForm::Layout()
{
	if (HasFocus())
	{
		std::string parent = "ROOT";
		Clay_String id = Clay_String{ .length = (int)parent.length(), .chars = parent.c_str() };
		CLAY(
			CLAY_ID_LOCAL("MAINFORM"),
			CLAY_FLOATING({ .parentId = Clay_GetElementId(id).id, .attachment = {.element = CLAY_ATTACH_POINT_CENTER_CENTER, .parent = CLAY_ATTACH_POINT_CENTER_CENTER } }),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(_layoutDimensions.width), .height = CLAY_SIZING_FIXED(_layoutDimensions.height)}, .padding = {.left = 16, .right = 16, .top = 8, .bottom = 8 }, .layoutDirection = CLAY_LEFT_TO_RIGHT }),
			CLAY_RECTANGLE({ .color = _config.colors.accent }),
			CLAY_BORDER_OUTSIDE({ .width = 2, .color = _config.colors.highlight })
		)
		{
			LayoutTitle();
		}
	}
}

void UI_Layout_SaveForm::SetFileExplorer()
{
	std::string current = _pathTxt->GetText();

	if (current.empty())
	{
		_pathTxt->SetText(_dir + "\\default.mk");
	}

	//see if the path exists
	if (!std::filesystem::exists(_dir))
	{
		std::filesystem::create_directory(_dir);
	}

	//enumerate files with .mk extension
	std::vector<std::string> files;

	for (const auto& entry : std::filesystem::directory_iterator(_dir))
	{
		if (entry.path().extension() == ".mk")
		{
			files.push_back(entry.path().filename().string());
		}
	}

	_filesArea->Clear();
	for (const auto& file : files)
	{
		_filesArea->PushElement(file);
	}
}

void UI_Layout_SaveForm::LayoutTitle()
{
	CLAY(
		CLAY_ID_LOCAL("OVERALL"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _config.colors.accent })
	)
	{
		CLAY(
			CLAY_ID_LOCAL("TITLECONTAINER"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)}, .layoutDirection = CLAY_LEFT_TO_RIGHT }),
			CLAY_RECTANGLE({ .color = _config.colors.accent })
		)
		{
			CLAY(
				CLAY_ID_LOCAL("TITLE"),
				CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)}, .layoutDirection = CLAY_LEFT_TO_RIGHT }),
				CLAY_RECTANGLE({ .color = _config.colors.accent })
			)
			{
				Clay_String name = Clay_StringFromStdString(_title);
				CLAY_TEXT(name, CLAY_TEXT_CONFIG({ .textColor = _config.colors.text, .fontId = _config.fontId, .fontSize = _config.fontSize }));
			}
			_closeBtn->Layout();
		}
		CLAY(
			CLAY_IDI_LOCAL("PAD", 3),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
			CLAY_RECTANGLE({ .color = _config.colors.accent })
		)
		{
		}

		CLAY(
			CLAY_ID_LOCAL("CONTENTS"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
			CLAY_RECTANGLE({ .color = _config.colors.accent })
		)
		{
			CLAY(
				CLAY_IDI_LOCAL("PAD", 0),
				CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
				CLAY_RECTANGLE({ .color = _config.colors.accent })
			)
			{
			}

			CLAY(
				CLAY_IDI_LOCAL("TEXTBORDER", 0),
				CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
				CLAY_RECTANGLE({ .color = _config.colors.accent }),
				CLAY_BORDER_OUTSIDE({ .width = 2, .color = _config.colors.accentText })
			)
			{
				_dirTxt->SetText(_dir);
				_dirTxt->Layout();
			}

			CLAY(
				CLAY_IDI_LOCAL("PAD", 1),
				CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
				CLAY_RECTANGLE({ .color = _config.colors.accent })
			)
			{
			}

			CLAY(
				CLAY_IDI_LOCAL("TEXTBORDER", 1),
				CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
				CLAY_RECTANGLE({ .color = _config.colors.accent }),
				CLAY_BORDER_OUTSIDE({ .width = 2, .color = _config.colors.accentText })
			)
			{
				_filesArea->Layout();
				if (_filesArea->GetSelectedIdx() >= 0)
				{
					_path = _dir + "\\" + _filesArea->GetSelected();
					_pathTxt->SetText(_path);
				}
			}

			CLAY(
				CLAY_IDI_LOCAL("PAD", 2),
				CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
				CLAY_RECTANGLE({ .color = _config.colors.accent })
			)
			{
			}

			CLAY(
				CLAY_IDI_LOCAL("TEXTBORDER", 2),
				CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
				CLAY_RECTANGLE({ .color = _config.colors.accent }),
				CLAY_BORDER_OUTSIDE({ .width = 2, .color = _config.colors.accentText })
			)
			{
				_pathTxt->Layout();
			}

			CLAY(
				CLAY_IDI_LOCAL("PAD", 3),
				CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
				CLAY_RECTANGLE({ .color = _config.colors.accent })
			)
			{
			}

			_saveBtn->Layout();

			CLAY(
				CLAY_IDI_LOCAL("PAD", 4),
				CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
				CLAY_RECTANGLE({ .color = _config.colors.accent })
			)
			{
			}
		}
	}
}

void UI_Layout_SaveForm::LayoutContent()
{
}
