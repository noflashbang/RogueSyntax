#pragma once

#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>
#include <functional>
#include "UIConfig.h"


class UI_MenuBar
{
public:
	UI_MenuBar(const UIConfig& config);
	~UI_MenuBar() = default;

	void Layout();

	void AddMenu(const std::string& name, const std::vector<std::string>& items)
	{
		_menu.push_back(std::make_pair(name, items));
	};

private:

	void CreateMenu();
	void CreateMenuButton(uint16_t index, const std::string& name, const std::vector<std::string>& items);
	void CreateMenuDropDown(const std::vector<std::string>& items);
	void CreateMenuDropDownButton(const std::string& name, uint16_t index);
	UIConfig _config;

	std::vector<std::pair<std::string, std::vector<std::string>>> _menu;
	std::string _menuIdActive;

};
