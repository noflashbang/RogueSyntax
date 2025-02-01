#include "InputForm.h"
#include <iostream>


InputForm::InputForm(const std::string& name, UIConfig config, std::shared_ptr<UIEventObserver<std::string>> focusChanged) : _config(config), _eventCurrentFocusObserver(focusChanged)
{
	_name = name;
	_textArea = std::make_unique<UI_TextArea>(_config, _name, _eventCurrentFocusObserver, std::make_unique<SimpleLineNumbering>(_config));
}

InputForm::~InputForm()
{
}

void InputForm::SetContent(const std::string& content)
{
	_textArea->SetText(content);
}

void InputForm::ProcessInputCommand(const InputCmd& cmd)
{
	if (_eventCurrentFocusObserver->GetEventData() != _name)
	{
		return;
	}
	_textArea->ProcessInputCommand(cmd);
}

void InputForm::Layout()
{

	if (Clay_Hovered() && IsMouseButtonDown(0))
	{
		//observe focus
		_eventCurrentFocusObserver->SetEventData(_name);
	}

	_textArea->SetLayoutDimensions(_layoutDimensions);
	//bool hasFocus = _eventCurrentFocusObserver->GetEventData() == _name;
	//_textArea->HasFocus() = hasFocus;
	_textArea->Layout();
}



