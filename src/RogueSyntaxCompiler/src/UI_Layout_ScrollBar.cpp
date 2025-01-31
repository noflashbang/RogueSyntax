#include "UI_Layout_Scrollbar.h"

UI_ScrollBar::UI_ScrollBar(const UIConfig& config, float min, float max, float initalValue, float step, ScrollBarType type) : _config(config), _min(min), _max(max), _value(initalValue), _step(step), _type(type)
{
}

void UI_ScrollBar::Layout()
{
	LayoutScrollbar();
}

void UI_ScrollBar::LayoutScrollbar()
{
	auto verticalSizing = Clay_Sizing{ .width = CLAY_SIZING_FIXED((float)_config.fontSize), .height = CLAY_SIZING_GROW(0) };
	auto horizontalSizing = Clay_Sizing{ .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize) };

	auto sizing = _type == ScrollBarType::SCROLLBAR_VERTICAL ? verticalSizing : horizontalSizing;
	auto layoutDirection = _type == ScrollBarType::SCROLLBAR_VERTICAL ? CLAY_TOP_TO_BOTTOM : CLAY_LEFT_TO_RIGHT;

	CLAY(
		CLAY_ID_LOCAL("SCROLLCONTAINER"),
		CLAY_LAYOUT({ .sizing = sizing, .layoutDirection = layoutDirection }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		auto bb = _layoutDimensions;
		float longDirectionActual = (_type == ScrollBarType::SCROLLBAR_VERTICAL ? bb.height : bb.width) - _config.fontSize - _config.fontSize;

		auto barFill = (_step / (_max - _min)) * longDirectionActual;
		barFill = std::clamp(barFill, 0.0f, longDirectionActual);

		auto scrollFill = (_value / (_max - _min)) * longDirectionActual;
		scrollFill = std::clamp(scrollFill, 0.0f, longDirectionActual- barFill);

		auto endFill = longDirectionActual - scrollFill - barFill;
		endFill = std::clamp(endFill, 0.0f, longDirectionActual - barFill);

		float barwidth = _config.fontSize;
		float barHeight = _config.fontSize;

		float scrollwidth = _config.fontSize;
		float scrollHeight = _config.fontSize;

		float endwidth = _config.fontSize;
		float endHeight = _config.fontSize;

		if (_type == ScrollBarType::SCROLLBAR_VERTICAL)
		{
			barHeight = barFill;
			scrollHeight = scrollFill;
			endHeight = endFill;
		}
		else
		{
			barwidth = barFill;
			scrollwidth = scrollFill;
			endwidth = endFill;
		}


		CLAY(
			CLAY_ID_LOCAL("BTNSTART"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)_config.fontSize), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
			CLAY_RECTANGLE({ .color = _config.colors.foreground })
		)
		{
			if (Clay_Hovered() && IsMouseButtonPressed(0))
			{
				if (_value > 0)
				{
					_value--;
				}
			}
		}
		CLAY(
			CLAY_ID_LOCAL("FILLSTART"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(scrollwidth), .height = CLAY_SIZING_FIXED(scrollHeight)} }),
			CLAY_RECTANGLE({ .color = _config.colors.text })
		)
		{
		}
		CLAY(
			CLAY_ID_LOCAL("BTNSCROLL"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(barwidth), .height = CLAY_SIZING_FIXED(barHeight)} }),
			CLAY_RECTANGLE({ .color = _config.colors.highlight })
		)
		{
			if (Clay_Hovered() && IsMouseButtonDown(0))
			{
				_scrolling = true;
			}

			if (IsMouseButtonReleased(0))
			{
				_scrolling = false;
			}

			if (_scrolling)
			{
				auto mouseDelta = GetMouseDelta();

				_mouseDeltaAccumulated += _type == ScrollBarType::SCROLLBAR_VERTICAL ? (mouseDelta.y / _config.fontSize) : (mouseDelta.x / _config.fontSize);
				if (_mouseDeltaAccumulated > 0.5)
				{
					if (_value < _max)
					{
						_value++;
					}
					_mouseDeltaAccumulated = 0.0f;
				}
				else if (_mouseDeltaAccumulated < -0.5)
				{
					if (_value > 0)
					{
						_value--;
					}
					_mouseDeltaAccumulated = 0.0f;
				}
			}
		}
		CLAY(
			CLAY_ID_LOCAL("FILLEND"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(endwidth), .height = CLAY_SIZING_FIXED(endHeight)} }),
			CLAY_RECTANGLE({ .color = _config.colors.text })
		)
		{
		}
		CLAY(
			CLAY_ID_LOCAL("BTNEND"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)_config.fontSize), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
			CLAY_RECTANGLE({ .color = _config.colors.foreground })
		)
		{
			if (Clay_Hovered() && IsMouseButtonPressed(0))
			{
				if (_value < _max)
				{
					_value++;
				}
			}
		}
	}
}
