#pragma once

#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>
#include <functional>
#include <algorithm>

#include "UI_Layout_Structs.h"
#include "UIConfig.h"

enum ScrollBarType
{
	SCROLLBAR_VERTICAL,
	SCROLLBAR_HORIZONTAL
};

class UI_ScrollBar
{
public:
	UI_ScrollBar(const UIConfig& config, float min, float max, float initalValue, float step, ScrollBarType type);
	~UI_ScrollBar() = default;

	void Layout();

	void SetLayoutDimensions(const LayoutDimensions& dim) { _layoutDimensions = dim; };
	
	void SetValue(float value) { _value = value; };
	float GetValue() { return _value; };

	void SetMin(float min) { _min = min; };
	float GetMin() { return _min; };

	void SetMax(float max) { _max = max; };
	float GetMax() { return _max; };

	void SetStep(float step) { _step = step; };
	float GetStep() { return _step; };

private:

	void LayoutScrollbar();

	ScrollBarType _type;
	float _value = 0;
	float _min = 0;
	float _max = 0;
	float _step = 0;
	bool _scrolling = false;

	float _mouseDeltaAccumulated = 0;

	LayoutDimensions _layoutDimensions{0,0};

	UIConfig _config;
};
