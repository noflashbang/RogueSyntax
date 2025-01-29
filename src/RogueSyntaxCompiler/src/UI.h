#pragma once
#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>
#include "clay_renderer_raylib.h"
#include <unordered_map>
#include <string>
#include <ranges>
#include <algorithm>
#include <format>
#include "InputCmd.h"
#include "InputForm.h"
#include "UIConfig.h"
#include "UI_Layout_Splitter.h"

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) { .x = vector.x, .y = vector.y }

struct MenuId
{
	std::string id;
	std::string name;
};

class UI
{
public:

	UI(UIConfig config);
	~UI();

	void DoLayout();

	void SetDetails(const std::string& details) { _details = details; }
	void SetOutput(const std::string& output) { _output = output; }
	void SetEditor(const std::string& editor) { _editor = editor; }
	void SetInfo(const std::string& info) { _info = info; }

protected:

	void CreateRoot();
	void CreateMenu();
	void CreateMenuButton(const MenuId& name, const std::vector<MenuId>& items);
	void CreateMenuDropDown(const std::vector<MenuId>& items);
	void CreateMenuDropDownButton(const MenuId& name);
	void CreateActionBar();
	void CreateMainForm();
	void CreateIDEForm();
	void CreateEditor();
	void CreateOutput();
	void CreateInfo();
	void CreateDetails();

	void CreateInputCommands();

private:
	
	UIConfig _config;

	//more complex ui elements
	InputForm _editorForm;
	InputForm _outputForm;
	InputForm _infoForm;
	
	//splitters
	std::unique_ptr<UI_Splitter> _mainFormSplitter;
	std::unique_ptr<UI_Splitter> _ideFormSplitter;

	double _mouseBtnOneDownTime = 0.0;

	std::string _formFocus;

	std::vector<InputCmd> _inputCmds;

	std::string _details;
	std::string _output;
	std::string _editor;
	std::string _info;

	std::vector<std::pair<MenuId,std::vector<MenuId>>> _menu;
	std::string _menuIdActive;
};
