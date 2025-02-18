#pragma once

#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>
#include <functional>
#include "UIConfig.h"
#include "UI_Layout_Event.h"


class UI_MenuBar
{
public:
	UI_MenuBar(const UIConfig& config, std::shared_ptr<UIEventObserver<std::string>> focusChanged);
	~UI_MenuBar() = default;

	void Layout();

	void AddMenu(const std::string& name, const std::vector<std::string>& items)
	{
		_menu.push_back(std::make_pair(name, items));

		//create an event handler for each item
		for (auto& ii : items)
		{
			_menuEvents[name + ii] = UI_Delegate<NoLock>();
		}
	};

	std::unique_ptr<ScopedConnection> Subscribe(const std::string& menu, const std::string& item, std::function<void()> handler)
	{
		auto& event = _menuEvents[menu + item];
		return event += handler;
	};

private:

	std::unordered_map<std::string, UI_Delegate<NoLock>> _menuEvents;

	void CreateMenu();
	void CreateMenuButton(uint16_t index, const std::string& name, const std::vector<std::string>& items);
	void CreateMenuDropDown(const std::string& name, const std::vector<std::string>& items);
	void CreateMenuDropDownButton(const std::string& name, const std::string& item, uint16_t index);
	UIConfig _config;

	std::shared_ptr<UIEventObserver<std::string>> _eventCurrentFocusObserver;
	std::vector<std::pair<std::string, std::vector<std::string>>> _menu;
	std::string _menuIdActive;

};
