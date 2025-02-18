#include "EditorForm.h"
#include <iostream>


EditorForm::EditorForm(const std::string& name, UIConfig config, std::shared_ptr<UIEventObserver<std::string>> focusChanged) : _config(config), _eventCurrentFocusObserver(focusChanged)
{
	_name = name;
	_textArea = std::make_unique<UI_TextArea>(_config, _name, _eventCurrentFocusAgent.Subscribe(), std::make_unique<SimpleLineNumbering>(_config));
}

EditorForm::~EditorForm()
{
}

void EditorForm::SetContent(const std::string& content)
{
	_textArea->SetText(content);
}

void EditorForm::ProcessInputCommand(const InputCmd& cmd)
{
	_textArea->ProcessInputCommand(cmd);

	if (_eventCurrentFocusObserver->GetEventData() != _name)
	{
		_eventCurrentFocusAgent.SetEventData("");
	}
}

void EditorForm::Layout()
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



