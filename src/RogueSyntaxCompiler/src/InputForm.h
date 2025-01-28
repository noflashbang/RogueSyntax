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

std::vector<std::string> GetLinesBySplitor(const std::string& text, char splitor);

class ILineNumberingStrategy
{
public:
	virtual void LayoutLineNumbering(size_t line_number) = 0;
	virtual ~ILineNumberingStrategy() = default;
};

class SimpleLineNumbering : public ILineNumberingStrategy
{
public:
	SimpleLineNumbering(UIConfig config);
	virtual ~SimpleLineNumbering() = default;

	virtual void LayoutLineNumbering(size_t line_number);
private:
	std::vector<std::string> _lineNumbers;
	UIConfig _config;
};

class ICursorStrategy
{
public:
	virtual ~ICursorStrategy() = default;
	virtual void LayoutCursor() = 0;
	virtual void Update(double dt) = 0;
};

class BarCursorStrategy : public ICursorStrategy
{
public:
	BarCursorStrategy(UIConfig config);
	virtual ~BarCursorStrategy() = default;
	virtual void LayoutCursor();
	virtual void Update(double dt) { _cursorBlinker.Update(dt); }

private:
	UIConfig _config;

	//cursor blinker
	Blinker _cursorBlinker = Blinker(0.5);
};

class HighlightCursorStrategy : public ICursorStrategy
{
public:
	HighlightCursorStrategy(UIConfig config);
	virtual void LayoutCursor();
	virtual void Update(double dt) { _cursorBlinker.Update(dt); }

private:
	UIConfig _config;

	//cursor blinker
	Blinker _cursorBlinker = Blinker(0.5);
};

class InputForm
{
public:
	InputForm(const std::string& name, UIConfig config);
	~InputForm();

	void SetContent(const std::string& content);
	void ProcessInputCommands(const std::vector<InputCmd>& cmds);
	const std::vector<std::string>& GetInputFormLines() { return _inputFormLines; };

	void Layout(bool hasFocus);
	const std::string& Name() { return _name; }

protected:

	ILineNumberingStrategy* _lineNumberingStrategy = nullptr;
	void LayoutLineNumbering(size_t line_number);

	ICursorStrategy* _cursorStrategy = nullptr;
	void LayoutCursor(bool hasFocus, size_t line_number, size_t index);

	void CreateLine(bool hasFocus, size_t line_number, const std::string_view line);
	void CreateChar(bool hasFocus, size_t line_number, size_t index, const char* character);
	void CreatePlaceHolderChar(bool hasFocus, size_t line_number, size_t index, const char* character);

private:

	

	//ui font and colors, sizes
	UIConfig _config;

	//current cursor position
	uint16_t _cursorLine = 0;
	uint16_t _cursorColumn = 0;

	//start of the highlight
	bool _highlighting = false;
	uint16_t _highlightLine = 0;
	uint16_t _highlightColumn = 0;

	//mouse hover position
	uint16_t _hoverLine = 0;
	uint16_t _hoverColumn = 0;

	const char _cursorPlaceholder = ' '; //used to draw a text layout when the cursor is at the end of a line, need a valid char* to draw

	void DeleteHighlightedText(std::vector<std::string>& data);
	void CopyHighlightedText(std::vector<std::string>& data);
	void InsertClipboardText(std::vector<std::string>& data);

	void SetClipboardText(const std::string& text);
	std::string _clipBoardText;

	std::vector<std::string> _inputFormLines;
	std::string _name;
};