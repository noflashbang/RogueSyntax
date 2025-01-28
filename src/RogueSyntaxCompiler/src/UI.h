#pragma once
#include "clay.h"
#include "clayex.h"
#include <raylib.h>
#include <raymath.h>
#include "clay_renderer_raylib.h"
#include <unordered_map>
#include <string>
#include <ranges>
#include <algorithm>
#include <format>

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) { .x = vector.x, .y = vector.y }

const uint32_t FONT_ID_BODY_24 = 0;
const uint32_t FONT_ID_BODY_16 = 1;

std::vector<std::string> GetLinesBySplitor(const std::string& text, char splitor);

struct MenuId
{
	std::string id;
	std::string name;
};

enum InputCmdType
{
	INPUT_INSERT,
	INPUT_NEWLINE,
	INPUT_DELETE,
	INPUT_CURSOR_LEFT,
	INPUT_CURSOR_RIGHT,
	INPUT_CURSOR_UP,
	INPUT_CURSOR_DOWN,
	INPUT_CURSOR_HOME,
	INPUT_CURSOR_END,
	INPUT_CURSOR_PAGEUP,
	INPUT_CURSOR_PAGEDOWN,
	INPUT_CURSOR_COPY,
	INPUT_CURSOR_CUT,
	INPUT_CURSOR_PASTE
};

#define FLAG_NORMAL (0u)
#define FLAG_HIGHLIGHT (1u)
#define FLAG_DELETE_FWD (0u)
#define FLAG_DELETE_BWD (1u)

struct InputCmd
{
	InputCmdType type;
	uint32_t flags;
};

class UI
{
public:

	UI(Palette colors, uint32_t fontId, uint32_t fontsize);
	~UI();

	void DoLayout();

	void SetDetails(const std::string& details) { _details = details; }
	void SetOutput(const std::string& output) { _output = output; }
	void SetEditor(const std::string& editor) { _editor = editor; }
	void SetInfo(const std::string& info) { _info = info; }

protected:

	void CreateLine(const std::string& context, size_t line_number, const std::string_view line, bool lineNumbers);
	void CreateChar(const std::string& context, size_t line_number, size_t index, const char* character);

	void CreateRoot();
	void CreateMenu();
	void CreateMenuButton(const MenuId& name, const std::vector<MenuId>& items);
	void CreateMenuDropDown(const std::vector<MenuId>& items);
	void CreateMenuDropDownButton(const MenuId& name);
	void CreateActionBar();
	void CreateMainForm();
	void CreateMainFormSpliter();
	void CreateIDEForm();
	void CreateEditor();
	void CreateIDEFormSpliter();
	void CreateOutput();
	void CreateInfo();
	void CreateDetails();

	void CreateInputCommands();
	void ProcessInputCommands();

private:
	Palette _colors;
	uint16_t _fontId;
	uint16_t _fontSize;
	//font sized based padding
	Clay_Padding _padding;


	//current cursor position
	uint16_t _cursorLine = 0;
	uint16_t _cursorColumn = 0;

	//start of the highlight
	bool _highlighting = false;
	uint16_t _highlightLine = 0;
	uint16_t _highlightColumn = 0;
	
	void DeleteHighlightedText();
	void CopyHighlightedText();
	void InsertClipboardText();

	void SetClipboardText(const std::string& text);
	std::string _clipBoardText;
		
	std::vector<std::string> _editorLines;
	std::vector<std::string> _outputLines;
	std::vector<std::string> _infoLines;

	float _editorHeight = 600;
	bool _editorSizing = false;
	float _infoWidth = 1300;
	bool _infoSizing = false;

	bool _cursorBlink = false;
	double _keyboardAccumulatedTime = 0.0;
	double _blinkAccumulatedTime = 0.0;

	const char _cursorPlaceholder = ' '; //used to draw a text layout when the cursor is at the end of a line, need a valid char* to draw
	std::string _formFocus;

	std::vector<InputCmd> _inputCmds;

	std::string _details;
	std::string _output;
	std::string _editor;
	std::string _info;
	std::vector<std::pair<MenuId,std::vector<MenuId>>> _menu;

	std::vector<std::string> _lineNumbers;
	std::string _menuIdActive;
};
