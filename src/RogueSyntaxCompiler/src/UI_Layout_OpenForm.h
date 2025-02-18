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
#include <filesystem>
#include "UI_Layout_Structs.h"
#include "UI_Layout_Cursor.h"
#include "UI_Layout_LineNumber.h"
#include "UI_Layout_Event.h"
#include "UI_Layout_ScrollBar.h"
#include "UI_Layout_Textbox.h"
#include "UI_Layout_Listbox.h"
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

	UI_Delegate<NoLock, const std::string&>& onFormClosed() { return _onClosed; };

	void SetFileExplorer();

	void Open()
	{
		_path = _pathTxt->GetText();
		Close();
	};

	void Close()
	{
		_eventCurrentFocusObserver->SetEventData("");
		_hadFocus = false;

		if (!_path.empty())
		{
			_onClosed(_path);
		}
	};

	void LayoutTitle();
	void LayoutContent();

private:

	UI_Delegate<NoLock, const std::string&> _onClosed;

	UIEventAgent<std::string> _eventCurrentFocus;
	std::shared_ptr<UIEventObserver<std::string>> _eventCurrentFocusObserver;
	LayoutDimensions _layoutDimensions;

	//elements
	std::unique_ptr<UI_Button> _closeBtn;
	std::unique_ptr<ScopedConnection> _closeBtnConnection;

	std::unique_ptr<UI_Textbox> _pathTxt;
	std::unique_ptr<UI_Textbox> _dirTxt;
	std::unique_ptr<UI_Listbox> _filesArea;

	std::unique_ptr<UI_Button> _openBtn;
	std::unique_ptr<ScopedConnection> _openBtnConnection;

	//ui font and colors, sizes
	UIConfig _config;
	std::string _name;
	std::string _title;
	
	std::string _path;
	std::string _dir;

	bool _hadFocus = false;
};
