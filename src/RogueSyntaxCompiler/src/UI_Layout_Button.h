#pragma once

#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>
#include <functional>
#include <algorithm>
#include "InputCmd.h"
#include "UI_Layout_Structs.h"
#include "UI_Layout_Event.h"
#include "UI_Layout_Cursor.h"
#include "UIConfig.h"

#define HIGHLIGHT_END 0xFFFF

class UI_Button
{
public:

	UI_Button(const UIConfig& config, const std::string name);

	~UI_Button() = default;

	void Layout();
	void LayoutButton();
	void ProcessInputCommand(const InputCmd& cmds);

	void SetLayoutDimensions(LayoutDimensions dim) { _layoutDimensions = dim; };
	void SetText(const std::string& text) { _text = text; };
	const std::string& GetText() { return _text; };

	UI_Delegate<NoLock>& onClick() { return _onClick; };

private:

	UI_Delegate<NoLock> _onClick;
	LayoutDimensions _layoutDimensions;
	UIConfig _config;
	std::string _name;
	std::string _text;
	bool _buttonIsClicked;
};

