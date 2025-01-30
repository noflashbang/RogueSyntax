#include "UI_Layout_LineNumber.h"

SimpleLineNumbering::SimpleLineNumbering(UIConfig config) : _config(config)
{
	for (size_t i = 0; i < 99; i++)
	{
		_lineNumbers.push_back(std::format("{:0>2}", i));
	}
}

void SimpleLineNumbering::LayoutLineNumbering(size_t line_number)
{
	CLAY(
		CLAY_IDI_LOCAL("LINENUM", line_number),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED((float)(_config.fontSize * 2)), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		if (_lineNumbers.size() > line_number)
		{
			auto strContent = Clay_StringFromStdString(_lineNumbers.at(line_number));
			CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({ .textColor = _config.colors.text, .fontId = _config.fontId, .fontSize = _config.fontSize }));
		}
	}
}
