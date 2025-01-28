#pragma once
#include "clay.h"
#include "clayex.h"
#include "InputCmd.h"
#include "UIConfig.h"
#include <string>
#include <ranges>
#include <vector>
#include <format>


std::vector<std::string> GetLinesBySplitor(const std::string& text, char splitor);

class InputForm
{
public:
	InputForm(const std::string& name, UIConfig config);
	~InputForm();

	void SetContent(const std::string& content);
	void ProcessInputCommands(const std::vector<InputCmd>& cmds);
	const std::vector<std::string>& GetInputFormLines() { return _inputFormLines; };

	void Layout(bool hasFocus);

protected:

	void CreateLine(bool hasFocus, size_t line_number, const std::string_view line, bool lineNumbers);
	void CreateChar(bool hasFocus, size_t line_number, size_t index, const char* character);

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

	std::vector<std::string> _lineNumbers;
};