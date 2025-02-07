#include "UI_Layout_OpenForm.h"


UI_Layout_OpenForm::UI_Layout_OpenForm(uint16_t width, uint16_t height, const std::string& name, UIConfig config, std::shared_ptr<UIEventObserver<std::string>> focusChanged) : _config(config), _eventCurrentFocusObserver(focusChanged)
{
	_eventCurrentFocus = UIEventAgent<std::string>();

	_layoutDimensions.width = width;
	_layoutDimensions.height = height;
	_name = name;
	_title = "Open File";
	_path = "";

	_closeBtn = std::make_unique<UI_Button>(config, "closeButton");
	_closeBtn->SetLayoutDimensions(LayoutDimensions{ (float)_config.fontSize, (float)_config.fontSize });
	_closeBtn->SetText("X");
	_closeBtnConnection = _closeBtn->onClick() += [this]() { this->Close(); };

	_pathTxt = std::make_unique<UI_Textbox>(config, "pathTxt", _eventCurrentFocus.Subscribe(), std::make_unique<BarCursorStrategy>(_config));
	_pathTxt->SetLayoutDimensions(LayoutDimensions((float)width, (float)_config.fontSize));

	_filesArea = std::make_unique<UI_TextArea>(config, "filesArea", _eventCurrentFocus.Subscribe(), nullptr);
	_filesArea->SetLayoutDimensions(LayoutDimensions( (float)width, (float)_config.fontSize * 15 ));

	_openBtn = std::make_unique<UI_Button>(config, "openButton");
	_openBtn->SetLayoutDimensions(LayoutDimensions{ (float)_config.fontSize * 10, (float)_config.fontSize });
	_openBtn->SetText("Open");
	_openBtnConnection = _openBtn->onClick() += [this]() { this->Open(); };

	SetFileExplorer();
}

UI_Layout_OpenForm::~UI_Layout_OpenForm()
{
}

void UI_Layout_OpenForm::ProcessInputCommand(const InputCmd& cmd)
{
	if (HasFocus())
	{
		_pathTxt->ProcessInputCommand(cmd);
		_filesArea->ProcessInputCommand(cmd);
	}
}

void UI_Layout_OpenForm::Layout()
{
	if (HasFocus())
	{
		std::string parent = "ROOT";
		Clay_String id = Clay_String{ .length = (int)parent.length(), .chars = parent.c_str() };
		CLAY(
			CLAY_ID_LOCAL("MAINFORM"),
			CLAY_FLOATING({ .parentId = Clay_GetElementId(id).id, .attachment = { .element = CLAY_ATTACH_POINT_CENTER_CENTER, .parent = CLAY_ATTACH_POINT_CENTER_CENTER }  }),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(_layoutDimensions.width), .height = CLAY_SIZING_FIXED(_layoutDimensions.height)}, .layoutDirection = CLAY_LEFT_TO_RIGHT }),
			CLAY_RECTANGLE({ .color = _config.colors.accent }),
			CLAY_BORDER_OUTSIDE({ .width = 2, .color = _config.colors.highlight })
		)
		{
			LayoutTitle();
		}
	}
}



void UI_Layout_OpenForm::SetFileExplorer()
{
	//get working directory
	std::string path = std::filesystem::current_path().string() +"\\" + "example";

	//see if the path exists
	if (!std::filesystem::exists(path))
	{
		std::filesystem::create_directory(path);
	}

	//enumerate files with .mk extension
	std::vector<std::string> files;

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (entry.path().extension() == ".mk")
		{
			files.push_back(entry.path().filename().string());
		}
	}

	std::string displayedFiles = "";
	for (const auto& file : files)
	{
		displayedFiles += file + '\n';
	}

	//set text area text
	_filesArea->SetText(displayedFiles);
}

void UI_Layout_OpenForm::LayoutTitle()
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
			CLAY_ID_LOCAL("CONTENTS"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
			CLAY_RECTANGLE({ .color = _config.colors.accent })
		)
		{
			CLAY(
				CLAY_ID_LOCAL("TEXTBORDER"),
				CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
				CLAY_RECTANGLE({ .color = _config.colors.accent }),
				CLAY_BORDER_OUTSIDE({ .width = 2, .color = _config.colors.highlight })
			)
			{
				_filesArea->Layout();
			}
			_pathTxt->Layout();
			_openBtn->Layout();
		}
	}
}

void UI_Layout_OpenForm::LayoutContent()
{
}


