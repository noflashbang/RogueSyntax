#pragma once

#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>
#include <functional>
#include <algorithm>
#include <numeric>
#include <ranges>
#include "InputCmd.h"
#include "UI_Layout_Util.h"
#include "UI_Layout_Structs.h"
#include "UI_Layout_Event.h"
#include "UI_Layout_Cursor.h"
#include "UI_Layout_LineNumber.h"
#include "UI_Layout_ScrollBar.h"
#include "UI_Layout_Textbox.h"
#include "UIConfig.h"

class UI_Listbox
{
public:

	UI_Listbox(const UIConfig& config, const std::string name, std::shared_ptr<UIEventObserver<std::string>> focusChanged, const std::vector<std::string>& elems);

	~UI_Listbox();

	void Layout();
	void ProcessInputCommand(const InputCmd& cmds);
	void SetLayoutDimensions(const LayoutDimensions& dim);

	bool HasFocus() { return _eventCurrentFocusObserver->GetEventData() == _name; };
	std::string GetSelected() { return _elements[_selectedIdx]; };
	
	void SetSelected(int16_t idx) { _selectedIdx = idx; };
	int16_t GetSelectedIdx() { return _selectedIdx; };

	uint16_t PushElement(const std::string& elem) { _elements.push_back(elem); return _elements.size() - 1; };
	void Clear() { _elements.clear(); };
	std::string GetElement(uint16_t idx) { return _elements[idx]; };
	uint16_t GetElementCount() { return _elements.size(); };

private:

	void LayoutListbox();

	std::vector<std::string> _elements;
	std::shared_ptr<UIEventObserver<std::string>> _eventCurrentFocusObserver;
	int16_t _selectedIdx = -1;
	std::string _name;
	
	LayoutDimensions _layoutDimensions{0,0};
	UIConfig _config;
};

