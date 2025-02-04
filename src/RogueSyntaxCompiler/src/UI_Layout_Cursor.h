#pragma once

#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>

#include "UI_Layout_Structs.h"
#include "UI_Layout_Event.h"
#include "UIConfig.h"


class ICursorStrategy
{
public:
	virtual ~ICursorStrategy() = default;
	virtual void LayoutCursor() = 0;
	virtual void Update(double dt) = 0;
};

class BarCursorStrategy : public ICursorStrategy
{
public:
	BarCursorStrategy(UIConfig config);
	virtual ~BarCursorStrategy() = default;
	virtual void LayoutCursor();
	virtual void Update(double dt) { _cursorBlinker.Update(dt); }

private:
	UIConfig _config;

	//cursor blinker
	Blinker _cursorBlinker = Blinker(0.5);
};

class HighlightCursorStrategy : public ICursorStrategy
{
public:
	HighlightCursorStrategy(UIConfig config);
	virtual void LayoutCursor();
	virtual void Update(double dt) { _cursorBlinker.Update(dt); }

private:
	UIConfig _config;

	//cursor blinker
	Blinker _cursorBlinker = Blinker(0.5);
};