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

class InputForm
{
public:
	InputForm(const std::string& name, UIConfig config, std::shared_ptr<UIEventObserver<std::string>> focusChanged);
	~InputForm();

	void SetContent(const std::string& content);
	void ProcessInputCommand(const InputCmd& cmd);

	void Layout();
	const std::string& Name() { return _name; };


	CursorPosition GetCursorPosition() { return _textArea->GetCursorPosition(); };
	CursorPosition GetHighlightPosition() { return _textArea->GetHighlightPosition(); };
	CursorPosition GetHoverPosition() { return _textArea->GetHoverPosition(); };
	bool IsHighlighting() { return _textArea->IsHighlighting(); };

	void SetLayoutDimensions(LayoutDimensions dim) { _layoutDimensions = dim;};

private:

	std::shared_ptr<UIEventObserver<std::string>> _eventCurrentFocusObserver;
	std::unique_ptr<UI_TextArea> _textArea;
	LayoutDimensions _layoutDimensions;

	//ui font and colors, sizes
	UIConfig _config;

	std::string _name;
};
