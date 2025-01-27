#pragma once
#include <vector>
#include "clay.h"
#include <string>

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) { .x = vector.x, .y = vector.y }

struct Palette
{
	Clay_Color  background;
	Clay_Color  text;
	Clay_Color  accent;
	Clay_Color  accentText;
	Clay_Color  foreground;
	Clay_Color  highlight;
};

constexpr Clay_Color inline Clay_ColorFromInt(uint32_t value)
{
	return Clay_Color{
		.r = (float)((value >> 16) & 255),
		.g = (float)((value >> 8) & 255),
		.b = (float)((value) & 255),
		.a = (float)(255)
	};
}

constexpr Clay_String Clay_StringFromStdString(const std::string& str)
{
	return { .length = (int32_t)str.length(), .chars = str.c_str() };
};