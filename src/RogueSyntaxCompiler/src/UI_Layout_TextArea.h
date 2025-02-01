#pragma once

#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>
#include <functional>
#include <algorithm>
#include <numeric>
#include <ranges>
#include "InputCmd.h"
#include "UI_Layout_Structs.h"
#include "UI_Layout_Event.h"
#include "UI_Layout_Cursor.h"
#include "UI_Layout_LineNumber.h"
#include "UI_Layout_ScrollBar.h"
#include "UI_Layout_Textbox.h"
#include "UIConfig.h"

std::vector<std::string> GetLinesBySplitor(const std::string& text, char splitor);

class UI_TextArea
{
public:

	UI_TextArea(const UIConfig& config, const std::string name, std::shared_ptr<UIEventObserver<std::string>> focusChanged, std::unique_ptr<ILineNumberingStrategy> numbering);

	~UI_TextArea() = default;

	void Layout();
	void ProcessInputCommand(const InputCmd& cmds);

	void SetLayoutDimensions(const LayoutDimensions& dim);
	void SetText(const std::string& text);
	const std::string& GetText();
	bool HasFocus() { return _hasFocus; };
	bool IsHighlighting() { return _highlighting; };
	void SetHighlighting(bool highlight) { _highlighting = highlight; };
	void SetCursorPosition(CursorPosition position) { _cursorPosition.line = std::clamp(position.line, (uint16_t)0, (uint16_t)_textboxes.size());
													  _cursorPosition.column = std::clamp(position.column, (uint16_t)0, (uint16_t)_textboxes.at(_cursorPosition.line)->GetText().size()); };

	CursorPosition GetCursorPosition() { return _cursorPosition; };
	CursorPosition GetHighlightPosition() { return _highlightPosition; };
	CursorPosition GetHoverPosition() { return _hoverPosition; };

private:
	uint32_t textboxId = 0;
	std::string TextboxName() { return _name + "_LN_" + std::to_string(++textboxId);};
	void LayoutWithScrollbars();
	void LayoutTextArea();
	void LayoutScrollbarH();
	void LayoutScrollbarV();
	void ScrollCursorIntoView();
	void NormalizeScrollOffset();

	uint16_t GetViewPortWidth();
	uint16_t GetViewPortHeight();

	void CreateLine(size_t index);

	std::string GetHighlightedText();
	void DeleteHighlightedText();
	void InsertText(const std::string& text);

	std::unique_ptr<ILineNumberingStrategy> _lineNumberingStrategy;

	UIEventAgent<std::string> _eventTextboxFocus;
	std::shared_ptr<UIEventObserver<std::string>> _eventCurrentFocusObserver;
	std::string _name;
	std::string _text;
	
	std::vector<std::unique_ptr<UI_Textbox>> _textboxes;
	bool _hasFocus = false;
	bool _highlighting = false;
	CursorPosition _cursorPosition;
	CursorPosition _hoverPosition;
	CursorPosition _highlightPosition;

	//scroll bars
	bool _allowScrolling = true;
	CursorPosition _scrollOffset{0,0};
	std::unique_ptr<UI_ScrollBar> _scrollBarH;
	std::unique_ptr<UI_ScrollBar> _scrollBarV;

	LayoutDimensions _layoutDimensions{0,0};
	UIConfig _config;

	std::string _copyBuffer;
};

