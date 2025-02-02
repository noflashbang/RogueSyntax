#include "OutputForm.h"
#include <iostream>


OutputForm::OutputForm(const std::string& name, UIConfig config, std::shared_ptr<UIEventObserver<std::string>> focusChanged) : _config(config), _eventCurrentFocusObserver(focusChanged)
{
	_name = name;
	_outputArea = std::make_unique<UI_TextArea>(_config, _name + "area", _eventCurrentFocusAgent.Subscribe(), nullptr);
	_cmdBox = std::make_unique<UI_Textbox>(_config, _name + "cmd", _eventCurrentFocusAgent.Subscribe(), std::make_unique<HighlightCursorStrategy>(_config));
	_onReturnConnection = _cmdBox->onReturn() += [this](const std::string& cmd) { _onReturn(cmd); };
}

OutputForm::~OutputForm()
{
}

void OutputForm::ProcessInputCommand(const InputCmd& cmd)
{
	_cmdBox->ProcessInputCommand(cmd);
	_outputArea->ProcessInputCommand(cmd);

	if (_eventCurrentFocusObserver->GetEventData() != _name)
	{
		_eventCurrentFocusAgent.SetEventData("");
	}
}

void OutputForm::Layout()
{
	if (Clay_Hovered() && IsMouseButtonDown(0))
	{
		//observe focus
		_eventCurrentFocusObserver->SetEventData(_name);
	}

	CLAY(
		CLAY_ID_LOCAL("OUTPUT_PARENT_AREA"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)}, .childAlignment = {.x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_BOTTOM},  .layoutDirection = CLAY_LEFT_TO_RIGHT }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		_outputArea->SetLayoutDimensions(_layoutDimensions);
		_outputArea->Layout();
	}
	CLAY(
		CLAY_ID_LOCAL("OUTPUT_PARENT"),
		CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)}, .childAlignment = {.x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_CENTER},  .layoutDirection = CLAY_LEFT_TO_RIGHT }),
		CLAY_RECTANGLE({ .color = _config.colors.background })
	)
	{
		CLAY(
			CLAY_ID_LOCAL("PROMPT"),
			CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIT(0), .height = CLAY_SIZING_FIXED((float)_config.fontSize)} }),
			CLAY_RECTANGLE({ .color = _config.colors.background })
		)
		{
			auto strContent = Clay_String{ .length = (int)_prompt.length(), .chars = _prompt.c_str()};
			CLAY_TEXT(strContent, CLAY_TEXT_CONFIG({.textColor = _config.colors.text, .fontId = _config.fontId, .fontSize = _config.fontSize}));	
		}
		_cmdBox->Layout();
	}
}



