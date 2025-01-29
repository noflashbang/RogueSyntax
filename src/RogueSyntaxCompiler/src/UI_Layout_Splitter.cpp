#include "UI_Layout_Splitter.h"

UI_Splitter::UI_Splitter(const UIConfig& config, float defaultSize, SplitterType type, std::function<void()> first, std::function<void()> second) : _type(type), _layoutFirst(first), _layoutSecond(second), _size(defaultSize), _sizing(false), _config(config)
{
}

void UI_Splitter::Layout()
{
	switch (_type)
	{
	case SplitterType::SPLITTER_VERTICAL:
		LayoutVertical();
		break;
	case SplitterType::SPLITTER_HORIZONTAL:
		LayoutHorizontal();
		break;
	}
}

void UI_Splitter::LayoutVertical()
{
	CLAY(
		CLAY_ID_LOCAL("VERTICAL_SPLITTER"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_LEFT_TO_RIGHT }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		CLAY(
			CLAY_ID_LOCAL("SPLITTER_FIRST"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(_size), .height = CLAY_SIZING_GROW(0)}}),
			CLAY_RECTANGLE({ .color = _config.colors.background })
		)
		{
			_layoutFirst();
		}
		LayoutSplitter();
		CLAY(
			CLAY_ID_LOCAL("SPLITTER_SECOND"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)} }),
			CLAY_RECTANGLE({ .color = _config.colors.background })
		)
		{
			_layoutSecond();
		}
	}
}

void UI_Splitter::LayoutHorizontal()
{
	CLAY(
		CLAY_ID_LOCAL("VERTICAL_SPLITTER"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}, .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		CLAY(
			CLAY_ID_LOCAL("SPLITTER_FIRST"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(_size)} }),
			CLAY_RECTANGLE({ .color = _config.colors.background })
		)
		{
			_layoutFirst();
		}

		LayoutSplitter();
		
		CLAY(
			CLAY_ID_LOCAL("SPLITTER_SECOND"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)} }),
			CLAY_RECTANGLE({ .color = _config.colors.background })
		)
		{
			_layoutSecond();
		}
	}
}

void UI_Splitter::LayoutSplitter()
{
	auto horizontal = Clay_Sizing{ .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(4) };
	auto vertical = Clay_Sizing{ .width = CLAY_SIZING_FIXED(4), .height = CLAY_SIZING_GROW(0) };

	CLAY(
		CLAY_ID_LOCAL("Splitter"),
		CLAY_LAYOUT({ .sizing = _type == SplitterType::SPLITTER_VERTICAL ? vertical : horizontal }),
		CLAY_RECTANGLE({ .color = _config.colors.text })
	)
	{
		bool hovered = Clay_Hovered();
		if (hovered)
		{
			SetMouseCursor(_type == SplitterType::SPLITTER_VERTICAL ? MOUSE_CURSOR_RESIZE_EW : MOUSE_CURSOR_RESIZE_NS);
		}

		if (hovered && IsMouseButtonDown(0))
		{
			_sizing = true;
		}

		if (_sizing)
		{
			SetMouseCursor(_type == SplitterType::SPLITTER_VERTICAL ? MOUSE_CURSOR_RESIZE_EW : MOUSE_CURSOR_RESIZE_NS);
			if (IsMouseButtonUp(0))
			{
				SetMouseCursor(MOUSE_CURSOR_DEFAULT);
				_sizing = false;
			}

			_size += _type == SplitterType::SPLITTER_VERTICAL ? GetMouseDelta().x : GetMouseDelta().y;
		}
	}
}
