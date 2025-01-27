#pragma once
#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>
#include "clay_renderer_raylib.h"
#include <unordered_map>
#include <string>

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) { .x = vector.x, .y = vector.y }

const uint32_t FONT_ID_BODY_24 = 0;
const uint32_t FONT_ID_BODY_16 = 1;


struct MenuId
{
	std::string id;
	std::string name;
};

class UI
{
public:

	UI(Palette colors, uint32_t fontId, uint32_t fontsize);
	~UI();

	void DoLayout();


protected:

	void CreateRoot();
	void CreateMenu();
	void CreateMenuButton(const MenuId& name, const std::vector<MenuId>& items);
	void CreateMenuDropDown(const std::vector<MenuId>& items);
	void CreateMenuDropDownButton(const MenuId& name);
	void CreateActionBar();
	void CreateMainForm();
	void CreateMainFormSpliter();
	void CreateIDEForm();
	void CreateEditor();
	void CreateIDEFormSpliter();
	void CreateOutput();
	void CreateInfo();
	void CreateDetails();

private:
	Palette _colors;
	uint16_t _fontId;
	uint16_t _fontSize;

	float _editorHeight = 600;
	bool _editorSizing = false;
	float _infoWidth = 300;
	bool _infoSizing = false;

	std::string _details;
	std::string _output;
	std::string _editor;
	std::string _info;
	std::vector<std::pair<MenuId,std::vector<MenuId>>> _menu;

	std::string _menuIdActive;
};
