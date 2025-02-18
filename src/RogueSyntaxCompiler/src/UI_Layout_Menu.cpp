#include "UI_Layout_Menu.h"

UI_MenuBar::UI_MenuBar(const UIConfig& config, std::shared_ptr<UIEventObserver<std::string>> focusChanged) : _config(config), _eventCurrentFocusObserver(focusChanged)
{
}

void UI_MenuBar::Layout()
{
	CreateMenu();
}

void UI_MenuBar::CreateMenu()
{
	CLAY(
		CLAY_ID_LOCAL("MENU"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize * 2)}, .childGap = _config.fontSize, .layoutDirection = CLAY_LEFT_TO_RIGHT }),
		CLAY_RECTANGLE({ .color = _config.colors.accent })
	)
	{
		auto index = 0;
		for (auto& menu : _menu)
		{
			CreateMenuButton(index, menu.first, menu.second);
			index++;
		}
	}
}

void UI_MenuBar::CreateMenuButton(uint16_t index, const std::string& name, const std::vector<std::string>& items)
{
	auto strContent = Clay_StringFromStdString(name);
	Clay_Color color = _config.colors.background;
	Clay_Color textColor = _config.colors.text;
	if (_menuIdActive == name && _eventCurrentFocusObserver->GetEventData() == name)
	{
		color = _config.colors.foreground;
		textColor = _config.colors.accentText;
	}

	CLAY(
		CLAY_IDI_LOCAL("HOVER_CONTAINER", index),
		CLAY_LAYOUT()
	)
	{
		uint32_t borderSize = 0;
		if (Clay_Hovered())
		{
			borderSize = 2;
		}

		CLAY(
			CLAY_IDI_LOCAL("MENU_OPTION_BUTTON", index),
			CLAY_LAYOUT({ .padding = _config.padding }),
			CLAY_BORDER_OUTSIDE({ .width = borderSize, .color = _config.colors.highlight }),
			CLAY_RECTANGLE({ .color = color, .cornerRadius = 5 })
		)
		{
			CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = textColor, .fontId = _config.fontId, .fontSize = _config.fontSize }));

			if (_menuIdActive == name && _eventCurrentFocusObserver->GetEventData() == name)
			{
				CreateMenuDropDown(name, items);
			}

			if (Clay_Hovered())
			{
				if (IsMouseButtonDown(0))
				{
					_menuIdActive = name;
					_eventCurrentFocusObserver->SetEventData(name);
				}
			}
		}
	}
}

void UI_MenuBar::CreateMenuDropDown(const std::string& name, const std::vector<std::string>& items)
{
	CLAY(
		CLAY_ID("MENU"),
		CLAY_FLOATING({.attachment = { .parent = CLAY_ATTACH_POINT_LEFT_BOTTOM }}),
		CLAY_LAYOUT({.padding = {0, 0, 8, 8 }}))
	{
		CLAY(
			CLAY_LAYOUT({.sizing = {.width = CLAY_SIZING_FIXED((float)(_config.fontSize * 8))	},.layoutDirection = CLAY_TOP_TO_BOTTOM,}),
			CLAY_BORDER_OUTSIDE({ .width = 2, .color = _config.colors.highlight }),
			CLAY_RECTANGLE({.color = _config.colors.background, .cornerRadius = CLAY_CORNER_RADIUS(8)}))
		{
			auto index = 0;
			for (auto& item : items)
			{
				CreateMenuDropDownButton(name, item, index);
				index++;
			}
		}
	}
}

void UI_MenuBar::CreateMenuDropDownButton(const std::string& name, const std::string& item, uint16_t index)
{
	auto strContent = Clay_StringFromStdString(item);
	CLAY(
		CLAY_IDI_LOCAL("HOVER_CONTAINER", index),
		CLAY_LAYOUT()
	)
	{
		bool hovered = Clay_Hovered();
		if (hovered)
		{
			if (IsMouseButtonDown(0))
			{
				_eventCurrentFocusObserver->SetEventData("");
				_menuEvents[name + item]();
			}
		}
		CLAY(
			CLAY_IDI_LOCAL("BUTTON", index),
			CLAY_LAYOUT({ .sizing = {.width = CLAY_SIZING_GROW(0) }, .padding = _config.padding }),
			CLAY_RECTANGLE({ .color = hovered ? _config.colors.foreground : _config.colors.background, .cornerRadius = 5 })
		)
		{
			CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = hovered ? _config.colors.accentText : _config.colors.text, .fontId = _config.fontId, .fontSize = _config.fontSize }));
		}
	}
}
