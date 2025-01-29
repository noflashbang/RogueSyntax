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

struct CursorPosition
{
	uint16_t line;
	uint16_t column;

	bool operator==(const CursorPosition& other) const
	{
		return line == other.line && column == other.column;
	}
};

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

class IInputCmdProcessor
{
public:
	virtual bool ProcessInputCommand(const InputCmd& cmd) = 0;
	virtual ~IInputCmdProcessor() = default;
};

class ConsoleInputCmdProcessor : public IInputCmdProcessor
{
public:
	ConsoleInputCmdProcessor();
	virtual ~ConsoleInputCmdProcessor() = default;
	virtual bool ProcessInputCommand(const InputCmd& cmd) override;

private:

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

	void LayoutScrollBar();


	CursorPosition GetCursorPosition() { return _cursorPosition; }
	CursorPosition GetHighlightPosition() { return _highlightPosition; }
	CursorPosition GetHoverPosition() { return _hoverPosition; }
	bool IsHighlighting() { return _cursorPosition != _highlightPosition && _highlighting; }

	void SetFormHeight(float height) { _formHeight = height; NormalizeScrollOffset(); }

protected:

	friend class InputFormConfigurator;

	ILineNumberingStrategy* _lineNumberingStrategy = nullptr;
	void LayoutLineNumbering(size_t line_number);

	ICursorStrategy* _cursorStrategy = nullptr;
	void LayoutCursor(bool hasFocus, size_t line_number, size_t index);


	void CreateLine(bool hasFocus, size_t line_number, const std::string_view line);
	void CreateChar(bool hasFocus, size_t line_number, size_t index, const char* character);
	void CreatePlaceHolderChar(bool hasFocus, size_t line_number, size_t index, const char* character);

	void NormalizeScrollOffset();
	void ScrollCursorIntoView();

private:

	float _formHeight = 0;
	float _mouseYDeltaAccumulated = 0;
	bool _scrolling = false;

	uint16_t _scrollOffset = 0;

	//ui font and colors, sizes
	UIConfig _config;

	//current cursor position
	CursorPosition _cursorPosition;

	//start of the highlight
	bool _highlighting = false;
	CursorPosition _highlightPosition;

	//mouse hover position
	CursorPosition _hoverPosition;

	const char _cursorPlaceholder = ' '; //used to draw a text layout when the cursor is at the end of a line, need a valid char* to draw

	void DeleteHighlightedText(std::vector<std::string>& data);
	void CopyHighlightedText(std::vector<std::string>& data);
	void InsertClipboardText(std::vector<std::string>& data);

	void SetClipboardText(const std::string& text);
	std::string _clipBoardText;

	std::vector<std::string> _inputFormLines;
	std::string _name;
};

class InputFormConfigurator
{
public:
	InputFormConfigurator(InputForm* toConfigure) : _target(toConfigure) {};
	virtual ~InputFormConfigurator() = default;

	InputFormConfigurator& With(ILineNumberingStrategy* strategy)
	{
		_target->_lineNumberingStrategy = strategy;	
		return *this;
	};

	InputFormConfigurator& With(ICursorStrategy* strategy)
	{
		_target->_cursorStrategy = strategy;
		return *this;
	};
	InputForm* _target;
};