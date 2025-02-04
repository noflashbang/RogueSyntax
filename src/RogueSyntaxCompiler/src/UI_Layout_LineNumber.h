#pragma once

#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>

#include <format>

#include "UI_Layout_Structs.h"
#include "UI_Layout_Event.h"
#include "UIConfig.h"

class ILineNumberingStrategy
{
public:
	virtual void LayoutLineNumbering(size_t line_number) = 0;
	virtual ~ILineNumberingStrategy() = default;
};

class SimpleLineNumbering : public ILineNumberingStrategy
{
public:
	SimpleLineNumbering(UIConfig config);
	virtual ~SimpleLineNumbering() = default;

	virtual void LayoutLineNumbering(size_t line_number);
private:
	std::vector<std::string> _lineNumbers;
	UIConfig _config;
};

