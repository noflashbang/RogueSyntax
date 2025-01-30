#include "UI_Layout_Cursor.h"

BarCursorStrategy::BarCursorStrategy(UIConfig config) : _config(config)
{
}

void BarCursorStrategy::LayoutCursor()
{
	if (_cursorBlinker)
	{
		CLAY(
			CLAY_ID_LOCAL("CURSOR"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(1), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
			CLAY_FLOATING({ .attachment = {.element = CLAY_ATTACH_POINT_LEFT_CENTER, .parent = CLAY_ATTACH_POINT_LEFT_CENTER } }),
			CLAY_RECTANGLE({ .color = _config.colors.highlight })
		)
		{
		}
	}
	else
	{
		CLAY(
			CLAY_ID_LOCAL("CURSOR"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(1), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
			CLAY_FLOATING({ .attachment = {.element = CLAY_ATTACH_POINT_LEFT_CENTER, .parent = CLAY_ATTACH_POINT_LEFT_CENTER } }),
			CLAY_RECTANGLE({  })
		)
		{
		}
	}
}

HighlightCursorStrategy::HighlightCursorStrategy(UIConfig config) : _config(config)
{
}

void HighlightCursorStrategy::LayoutCursor()
{
	if (_cursorBlinker)
	{
		Clay_Color cursorColor = _config.colors.highlight;
		cursorColor.a = 128;

		CLAY(
			CLAY_ID_LOCAL("CURSOR"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)(_config.fontSize / 2)), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
			CLAY_FLOATING({ .attachment = {.element = CLAY_ATTACH_POINT_LEFT_CENTER, .parent = CLAY_ATTACH_POINT_LEFT_CENTER } }),
			CLAY_RECTANGLE({ .color = cursorColor })
		)
		{
		}
	}
	else
	{
		CLAY(
			CLAY_ID_LOCAL("CURSOR"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(1), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
			CLAY_FLOATING({ .attachment = {.element = CLAY_ATTACH_POINT_LEFT_CENTER, .parent = CLAY_ATTACH_POINT_LEFT_CENTER } }),
			CLAY_RECTANGLE({  })
		)
		{
		}
	}
}