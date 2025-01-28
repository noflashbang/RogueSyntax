#pragma once
#include "clay.h"
#include "clayex.h"
#include "InputCmd.h"
#include <string>
#include <ranges>
#include <vector>
#include <format>


std::vector<std::string> GetLinesBySplitor(const std::string& text, char splitor);

class InputForm
{
public:
	InputForm(const std::string& name);
	~InputForm();

	void SetContent(const std::string& content);
	void ProcessInputCommands(const std::vector<InputCmd>& cmds);
	const std::vector<std::string>& GetInputFormLines() { return _inputFormLines; };

	void Layout();

protected:

	void CreateLine(const std::string& context, size_t line_number, const std::string_view line, bool lineNumbers);
	void CreateChar(const std::string& context, size_t line_number, size_t index, const char* character);

private:

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

	void DeleteHighlightedText(std::vector<std::string>& data);
	void CopyHighlightedText(std::vector<std::string>& data);
	void InsertClipboardText(std::vector<std::string>& data);

	void SetClipboardText(const std::string& text);
	std::string _clipBoardText;

	std::vector<std::string> _inputFormLines;
	std::string _name;

	std::vector<std::string> _lineNumbers;
};