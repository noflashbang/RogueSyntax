#include "UI_Layout_Button.h"

UI_Button::UI_Button(const UIConfig& config, const std::string name) : _config(config), _name(name)
{
	_text = "";
}

void UI_Button::Layout()
{
	LayoutButton();
}

void UI_Button::ProcessInputCommand(const InputCmd& cmd)
{
	//NOOP
}


void UI_Button::LayoutButton()
{
	CLAY(
		CLAY_ID_LOCAL("BUTTON"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)_config.fontSize), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
		CLAY_RECTANGLE({ .color = _config.colors.highlight })
	)
	{
		Clay_String name = Clay_StringFromStdString(_text);
		CLAY_TEXT(name, CLAY_TEXT_CONFIG({ .textColor = _config.colors.background, .fontId = _config.fontId, .fontSize = _config.fontSize }));
		if (Clay_Hovered() && IsMouseButtonDown(0))
		{
			_buttonIsClicked = true;
		}

		if (Clay_Hovered() && IsMouseButtonReleased(0))
		{
			_buttonIsClicked = false;
			_onClick();
		}
	}
}
