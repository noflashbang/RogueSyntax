#pragma once
#include "clay.h"
#include "clayex.h"
#include "InputCmd.h"
#include "UIConfig.h"
#include <raylib.h> //timer
#include <string>
#include <ranges>
#include <vector>
#include <format>
#include "UI_Layout_Structs.h"
#include "UI_Layout_Cursor.h"
#include "UI_Layout_LineNumber.h"
#include "UI_Layout_Event.h"
#include "UI_Layout_ScrollBar.h"
#include "UI_Layout_Textbox.h"
#include "UI_Layout_TextArea.h"

class OutputForm
{
public:
	OutputForm(const std::string& name, UIConfig config, std::shared_ptr<UIEventObserver<std::string>> focusChanged);
	~OutputForm();

	void SetPrompt(const std::string& prompt) { _prompt = prompt; };
	
	std::string GetCommand() { return _cmdBox->GetText(); };
	std::string GetOutput() { return _outputArea->GetText(); };

	void ProcessInputCommand(const InputCmd& cmd);

	void Layout();
	const std::string& Name() { return _name; };

	void SetLayoutDimensions(LayoutDimensions dim) { _layoutDimensions = dim;};

private:
	UIEventAgent<std::string> _eventCurrentFocusAgent;
	std::shared_ptr<UIEventObserver<std::string>> _eventCurrentFocusObserver;
	std::unique_ptr<UI_TextArea> _outputArea;
	std::unique_ptr<UI_Textbox> _cmdBox;

	LayoutDimensions _layoutDimensions;

	//ui font and colors, sizes
	UIConfig _config;

	std::string _name;
	std::string _prompt;
	std::string _cmd;
	std::string _history;
};
