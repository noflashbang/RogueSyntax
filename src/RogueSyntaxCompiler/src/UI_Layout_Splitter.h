#pragma once

#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>
#include <functional>
#include "UIConfig.h"

enum SplitterType
{
	SPLITTER_VERTICAL,
	SPLITTER_HORIZONTAL
};

class UI_Splitter
{
public:
	UI_Splitter(const UIConfig& config, float defaultSize, SplitterType type, std::function<void()> first, std::function<void()> second);
	~UI_Splitter() = default;

	void Layout();

private:

	void LayoutVertical();
	void LayoutHorizontal();
	void LayoutSplitter();

	std::function<void()> _layoutFirst;
	std::function<void()> _layoutSecond;

	SplitterType _type;
	float _size = 600;
	bool _sizing = false;
	UIConfig _config;
};
