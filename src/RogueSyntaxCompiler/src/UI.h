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
#include "UIConfig.h"
#include "UI_Layout_Structs.h"
#include "UI_Layout_Splitter.h"
#include "UI_Layout_Menu.h"
#include "UI_Layout_Event.h"
#include "UI_Layout_Textbox.h"

#include "InfoForm.h"
#include "OutputForm.h"
#include "EditorForm.h"

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) { .x = vector.x, .y = vector.y }

class UI
{
public:

	UI(UIConfig config);
	~UI();

	void DoLayout();

	void SetDetails(const std::string& details) { _infoForm.SetContent(details); }
	void SetOutput(const std::string& output) { _outputForm.SetPrompt(output); }
	void SetEditor(const std::string& editor) { _editorForm.SetContent(editor); }

	void SetInfo(const std::string& info) { _info = info; }

protected:

	void CreateRoot();
	void CreateActionBar();
	void CreateMainForm();
	void CreateIDEForm();
	void CreateEditor();
	void CreateOutput();
	void CreateInfo();
	void CreateDetails();

	void CreateInputCommands();

	void SetLastCmd(const std::string& cmd) { LastCmd = cmd; _infoForm.SetContent(LastCmd); }
	std::unique_ptr<ScopedConnection> _onReturnConnection;
	std::string LastCmd = "";

private:
	
	UIConfig _config;
	UIEventAgent<std::string> _eventCurrentFocus;

	//more complex ui elements
	EditorForm _editorForm;
	OutputForm _outputForm;
	InfoForm _infoForm;

	//splitters
	std::unique_ptr<UI_Splitter> _mainFormSplitter;
	std::unique_ptr<UI_Splitter> _ideFormSplitter;

	//menu
	std::unique_ptr<UI_MenuBar> _menuBar;

	double _mouseBtnOneDownTime = 0.0;

	std::vector<InputCmd> _inputCmds;

	std::string _details;
	std::string _output;
	std::string _editor;
	std::string _info;
};
