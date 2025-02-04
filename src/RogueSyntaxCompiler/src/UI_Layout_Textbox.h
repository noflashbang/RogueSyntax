#pragma once

#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>
#include <functional>
#include <algorithm>
#include "InputCmd.h"
#include "UI_Layout_Structs.h"
#include "UI_Layout_Event.h"
#include "UI_Layout_Cursor.h"
#include "UIConfig.h"

#define HIGHLIGHT_END 0xFFFF

class UI_Textbox
{
public:

	UI_Textbox(const UIConfig& config, const std::string name, std::shared_ptr<UIEventObserver<std::string>> focusChanged, std::unique_ptr<ICursorStrategy> cursorType);

	~UI_Textbox() = default;

	void Layout();
	void Layout(uint16_t offset, uint16_t length);
	void ProcessInputCommand(const InputCmd& cmds);

	void SetLayoutDimensions(LayoutDimensions dim) { _layoutDimensions = dim; };
	void SetText(const std::string& text) { _text = text; UpdateText(); };
	const std::string& GetText() { return _text; };
	bool HasFocus() { return _eventCurrentFocusObserver->GetEventData() == _name; };
	void SetFocus();
	bool IsHighlighting() { return _highlighting; };
	void SetHighlighting(bool highlight) { _highlighting = highlight; };
	void SetHighlightingPosition(uint16_t start, uint16_t end);
	void SetCursorPosition(uint16_t position) { _cursorPosition = std::clamp(position, (uint16_t)0, (uint16_t)_text.size()); };
	uint16_t GetCursorPosition() { return _cursorPosition; };
	uint16_t GetHoverPosition() { return _hoverPosition; };

	bool CursorAtStart() { return _cursorPosition == 0; };
	bool CursorAtEnd() { return _cursorPosition == _text.size(); };
	std::string GetHighlightedText();
	void DeleteHighlightedText();
	void InsertText(const std::string& text);

	void UpdateText() { _bindableText.set(_text); };
	UI_Delegate<NoLock,const std::string&>& onTextChanged() { return _bindableText.onChange(); };
	UI_Delegate<NoLock, const std::string&>& onReturn() { return _onReturn; };


private:
	void LayoutCursor(size_t index);
	void LayoutTextbox(uint16_t offset, uint16_t length);

	void CreateChar(size_t index, const char* character);
	void CreatePlaceHolderChar(size_t index, const char* character);


	static const char _cursorPlaceholder = ' '; //used to draw a text layout when the cursor is at the end of a line, need a valid char* to draw
	std::unique_ptr<ICursorStrategy> _cursorStrategy;
	std::shared_ptr<UIEventObserver<std::string>> _eventCurrentFocusObserver;
	std::string _name;
	
	std::string _text;
	Bindable<NoLock, std::string> _bindableText = Bindable<NoLock, std::string>("");
	UI_Delegate<NoLock, const std::string&> _onReturn;

	bool _highlighting = false;
	uint16_t _cursorPosition = 0;
	uint16_t _hoverPosition;
	uint16_t _highlightPosition;

	LayoutDimensions _layoutDimensions;
	UIConfig _config;

	std::string _copyBuffer;
};

