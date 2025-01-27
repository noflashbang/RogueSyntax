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
#include <coroutine>

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) { .x = vector.x, .y = vector.y }

const uint32_t FONT_ID_BODY_24 = 0;
const uint32_t FONT_ID_BODY_16 = 1;


template <typename R>
auto enumerate_lines(R&& range)
{
	struct enumerate_iterator
	{
		using base_iterator = decltype(std::begin(range));
		enumerate_iterator(base_iterator iter, base_iterator endIter) : it(iter), end(endIter), newLineOrEnd(iter), index(0)
		{
			while (!(newLineOrEnd == end || *newLineOrEnd == '\n'))
			{
				++newLineOrEnd;
			}
		}

		base_iterator it;
		base_iterator newLineOrEnd;
		base_iterator end;

		size_t index;

		bool operator!=(const enumerate_iterator& other) const
		{
			return it != other.it;
		}

		void operator++()
		{
			++index;

			if (newLineOrEnd != end)
			{
				newLineOrEnd++;
			}

			it = newLineOrEnd;

			while (!(newLineOrEnd == end || *newLineOrEnd == '\n'))
			{
				++newLineOrEnd;
			}
		}

		auto operator*() const
		{
			return std::pair{ index, std::string_view(it, newLineOrEnd)};
		}
	};

	struct enumerate_view : public std::ranges::view_interface<enumerate_view>
	{
		R range;

		//solves error C2440 - not sure why the compiler (MSVC) needs this
		enumerate_view(R&& range) : range(std::forward<R>(range)) {};

		auto begin()
		{
			return enumerate_iterator{ std::begin(range), std::end(range) };
		}

		auto end()
		{
			return enumerate_iterator{ std::end(range), std::end(range) };
		}
	};

	return enumerate_view{ std::forward<R>(range) };
}

template <typename R>
auto enumerate_chars(R&& range)
{
	struct enumerate_iterator
	{
		using base_iterator = decltype(std::begin(range));
		base_iterator it;
		size_t index = 0;

		bool operator!=(const enumerate_iterator& other) const
		{
			return it != other.it;
		}

		void operator++() 
		{
			++it;
			++index;
		}

		auto operator*() const 
		{
			return std::pair{ index, &(*it) };
		}
	};

	struct enumerate_view : public std::ranges::view_interface<enumerate_view>
	{
		R range;

		//solves error C2440 - not sure why the compiler (MSVC) needs this
		enumerate_view(R&& range) : range(std::forward<R>(range)) {};

		auto begin() 
		{
			return enumerate_iterator{ std::begin(range) };
		}

		auto end() 
		{
			return enumerate_iterator{ std::end(range), std::ranges::size(range)+1 };
		}
	};

	return enumerate_view{ std::forward<R>(range) };
}

//line enumerator
struct LineEnumerator 
{
	struct promise_type 
	{
		std::pair<size_t, std::string_view> line_info;
		std::suspend_always yield_value(std::pair<size_t, std::string_view> new_line_info) 
		{
			line_info = std::move(new_line_info);
			return {};
		}
		std::suspend_never initial_suspend() { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		LineEnumerator get_return_object() { return LineEnumerator{ std::coroutine_handle<promise_type>::from_promise(*this) }; }
		void return_void() {}
		void unhandled_exception() { std::terminate(); }
	};

	std::coroutine_handle<promise_type> handle;
	LineEnumerator(std::coroutine_handle<promise_type> h) : handle(h) {}
	~LineEnumerator() { if (handle) handle.destroy(); }
	bool move_next()
	{
		if (handle.done())
			return false;

		handle.resume();
		return !handle.done();
//		bool done = handle.done();
//		if (!done)
//			handle.resume();
//		return !done; 
	}
	std::pair<size_t, std::string_view> current_value() { return handle.promise().line_info; }
};

//char enumerator
struct CharEnumerator 
{
	struct promise_type 
	{
		std::tuple<size_t, size_t, const char*> char_info;
		std::suspend_always yield_value(std::tuple<size_t, size_t, const char*> new_char_info) 
		{
			char_info = std::move(new_char_info);
			return {};
		}
		std::suspend_never initial_suspend() { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		CharEnumerator get_return_object() { return CharEnumerator{ std::coroutine_handle<promise_type>::from_promise(*this) }; }
		void return_void() {}
		void unhandled_exception() { std::terminate(); }
	};

	std::coroutine_handle<promise_type> handle;
	CharEnumerator(std::coroutine_handle<promise_type> h) : handle(h) {}
	~CharEnumerator() { if (handle) handle.destroy(); }
	bool move_next()
	{
		if (handle.done())
			return false;

		handle.resume();
		return !handle.done();
		//bool done = handle.done();
		//if (!done)
		//	handle.resume();
		//return !done;
	}
	std::tuple<size_t, size_t, const char*> current_value() { return handle.promise().char_info; }
};

struct MenuId
{
	std::string id;
	std::string name;
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

	//coroutines
	LineEnumerator EnumerateLines(const std::string& text);
	CharEnumerator EnumerateChars(size_t line_number, const std::string_view& line);

	void CreateLine(size_t line_number, const std::string_view& line);
	void CreateChar(size_t line_number, size_t index, const char* character);

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

private:
	Palette _colors;
	uint16_t _fontId;
	uint16_t _fontSize;

	float _editorHeight = 600;
	bool _editorSizing = false;
	float _infoWidth = 300;
	bool _infoSizing = false;

	bool _cursorBlink = false;
	double _blinkAccumulatedTime = 0.0;

	std::string _formFocus;

	std::string _details;
	std::string _output;
	std::string _editor;
	std::string _info;
	std::vector<std::pair<MenuId,std::vector<MenuId>>> _menu;

	std::string _menuIdActive;
};
