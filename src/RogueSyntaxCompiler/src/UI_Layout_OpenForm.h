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
#include "UI_Layout_Button.h"

class UI_Layout_OpenForm
{
public:
	UI_Layout_OpenForm(uint16_t width, uint16_t height, const std::string& name, UIConfig config, std::shared_ptr<UIEventObserver<std::string>> focusChanged);
	~UI_Layout_OpenForm();

	void ProcessInputCommand(const InputCmd& cmd);
	void Layout();
	const std::string& Name() { return _name; };
	void SetLayoutDimensions(LayoutDimensions dim) { _layoutDimensions = dim;};
	bool HasFocus()
	{
		if (!_hadFocus) { _hadFocus = _eventCurrentFocusObserver->GetEventData() == _name; }
		return _hadFocus || _eventCurrentFocusObserver->GetEventData() == _name;
	};



	void Close()
	{
		_eventCurrentFocusObserver->SetEventData("");
		_hadFocus = false;
	};

	void LayoutTitle();
	void LayoutContent();

private:

	std::shared_ptr<UIEventObserver<std::string>> _eventCurrentFocusObserver;
	LayoutDimensions _layoutDimensions;

	//elements
	std::unique_ptr<UI_Button> _closeBtn;
	std::unique_ptr<ScopedConnection> _closeBtnConnection;

	//ui font and colors, sizes
	UIConfig _config;
	std::string _name;
	std::string _title;
	bool _hadFocus = false;
};
