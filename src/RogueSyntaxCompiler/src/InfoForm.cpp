#include "InfoForm.h"
#include <iostream>


InfoForm::InfoForm(const std::string& name, UIConfig config, std::shared_ptr<UIEventObserver<std::string>> focusChanged) : _config(config), _eventCurrentFocusObserver(focusChanged)
{
	_name = name;
	_textArea = std::make_unique<UI_TextArea>(_config, _name, _eventCurrentFocusAgent.Subscribe(), nullptr);
}

InfoForm::~InfoForm()
{
}

void InfoForm::SetContent(const std::string& content)
{
	_textArea->SetText(content);
}

void InfoForm::ProcessInputCommand(const InputCmd& cmd)
{
	_textArea->ProcessInputCommand(cmd);

	if (_eventCurrentFocusObserver->GetEventData() != _name)
	{
		_eventCurrentFocusAgent.SetEventData("");
	}
}

void InfoForm::Layout()
{

	if (Clay_Hovered() && IsMouseButtonDown(0))
	{
		//observe focus
		_eventCurrentFocusObserver->SetEventData(_name);
	}
	CLAY(
		CLAY_ID_LOCAL("INFO_PARENT_AREA"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}, .childAlignment = {.x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_BOTTOM},  .layoutDirection = CLAY_TOP_TO_BOTTOM }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		_textArea->SetLayoutDimensions(_layoutDimensions);
		//bool hasFocus = _eventCurrentFocusObserver->GetEventData() == _name;
		//_textArea->HasFocus() = hasFocus;
		_textArea->Layout();
	}
}



