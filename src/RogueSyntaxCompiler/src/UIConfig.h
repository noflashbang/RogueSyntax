#pragma once
#include "clay.h"
#include "clayex.h"

const uint32_t FONT_ID_BODY_24 = 0;
const uint32_t FONT_ID_BODY_16 = 1;

struct UIConfig
{
	Palette colors;
	uint16_t fontId;
	uint16_t fontSize;

	//font sized based padding
	Clay_Padding padding;

	static UIConfig MakeDefault()
	{
		UIConfig config;
		config.colors.background = Clay_ColorFromInt(0x282c34);
		config.colors.text =       Clay_ColorFromInt(0xabb2bf);
		config.colors.accent =     Clay_ColorFromInt(0x61afef);
		config.colors.accentText = Clay_ColorFromInt(0x61afef);
		config.colors.foreground = Clay_ColorFromInt(0x3e4452);
		config.colors.highlight =  Clay_ColorFromInt(0x3e4452);
		config.fontId = FONT_ID_BODY_24;
		config.fontSize = 24;

		uint16_t half = config.fontSize / 2;
		config.padding = { config.fontSize, config.fontSize, half, half };

		return config;
	}

	UIConfig& operator=(const UIConfig& other)
	{
		colors = other.colors;
		fontId = other.fontId;
		fontSize = other.fontSize;
		padding = other.padding;
		return *this;
	}

	UIConfig(const UIConfig& other)
	{
		colors = other.colors;
		fontId = other.fontId;
		fontSize = other.fontSize;
		padding = other.padding;
	}

	~UIConfig() = default;

private:
	//private so that the factory method is the only way to create a UIConfig (other than copy/assign)
	UIConfig() = default;
};