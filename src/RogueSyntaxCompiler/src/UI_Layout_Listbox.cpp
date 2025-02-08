#include "UI_Layout_Listbox.h"

UI_Listbox::UI_Listbox(const UIConfig& config, const std::string name, std::shared_ptr<UIEventObserver<std::string>> focusChanged, const std::vector<std::string>& elems) : 
	_config(config), _name(name), _eventCurrentFocusObserver(focusChanged), _elements(elems)
{
}

UI_Listbox::~UI_Listbox()
{
}

void UI_Listbox::Layout()
{
	LayoutListbox();
}

void UI_Listbox::ProcessInputCommand(const InputCmd& cmds)
{
	if (HasFocus())
	{
		//noop
	}
}

void UI_Listbox::SetLayoutDimensions(const LayoutDimensions& dim)
{
	_layoutDimensions = dim;
}

void UI_Listbox::LayoutListbox()
{
	CLAY(
		CLAY_ID_LOCAL("LISTBOX"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)_layoutDimensions.width), .height = CLAY_SIZING_FIXED((float)_layoutDimensions.height)}, .padding = {.left = 16, .right = 16, .top = 8, .bottom = 8 }, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		auto iter = 0;
		for (auto& str : _elements)
		{
			Clay_String name = Clay_StringFromStdString(str);

			if (iter == GetSelectedIdx())
			{
				CLAY(
					CLAY_IDI_LOCAL("LINE", iter),
					CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)}}),
					CLAY_RECTANGLE({ .color = _config.colors.highlight })
				)
				{
					if (Clay_Hovered() && IsMouseButtonDown(0))
					{
						_selectedIdx = iter;
					}
					CLAY_TEXT(name, CLAY_TEXT_CONFIG({ .textColor = _config.colors.background, .fontId = _config.fontId, .fontSize = _config.fontSize }));
				}
			}
			else
			{
				CLAY(
					CLAY_IDI_LOCAL("LINE", iter),
					CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
					CLAY_RECTANGLE({ .color = _config.colors.background })
				)
				{
					if (Clay_Hovered() && IsMouseButtonDown(0))
					{
						_selectedIdx = iter;
					}
					CLAY_TEXT(name, CLAY_TEXT_CONFIG({ .textColor = _config.colors.text, .fontId = _config.fontId, .fontSize = _config.fontSize }));
				}
			}
			iter++;
		}
	}
}