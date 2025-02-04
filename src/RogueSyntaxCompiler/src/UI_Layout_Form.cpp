#include "UI_Layout_Form.h"


UI_Layout_Form::UI_Layout_Form(uint16_t width, uint16_t height, const std::string& name, UIConfig config, std::shared_ptr<UIEventObserver<std::string>> focusChanged) : _config(config), _eventCurrentFocusObserver(focusChanged)
{
	_layoutDimensions.width = width;
	_layoutDimensions.height = height;
	_name = name;
}

UI_Layout_Form::~UI_Layout_Form()
{
}

void UI_Layout_Form::ProcessInputCommand(const InputCmd& cmd)
{
	//_textArea->ProcessInputCommand(cmd);

	if (_eventCurrentFocusObserver->GetEventData() != _name)
	{
	}
}

void UI_Layout_Form::Layout()
{
	if (HasFocus())
	{
		std::string parent = "ROOT";
		Clay_String id = Clay_String{ .length = (int)parent.length(), .chars = parent.c_str() };
		CLAY(
			CLAY_ID_LOCAL("MAINFORM"),
				CLAY_FLOATING({ .parentId = Clay_GetElementId(id).id, .attachment = { .element = CLAY_ATTACH_POINT_CENTER_CENTER, .parent = CLAY_ATTACH_POINT_CENTER_CENTER }  }),
				CLAY_LAYOUT({ .sizing = Clay_Sizing{.width = CLAY_SIZING_FIXED(_layoutDimensions.width), .height = CLAY_SIZING_FIXED(_layoutDimensions.height)}, .layoutDirection = CLAY_LEFT_TO_RIGHT }),
				CLAY_RECTANGLE({ .color = _config.colors.accent }),
				CLAY_BORDER_OUTSIDE({ .width = 2, .color = _config.colors.highlight })
			)
			{
			//if (Clay_Hovered() && IsMouseButtonDown(0))
			//{
			//	//observe focus
			//	_eventCurrentFocusObserver->SetEventData(_name);
			//}
			}
	}
	//_textArea->SetLayoutDimensions(_layoutDimensions);
	//_textArea->Layout();
}



