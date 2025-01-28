#pragma once
#include <vector>
#include "clay.h"
#include <string>

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) { .x = vector.x, .y = vector.y }

struct Palette
{
	Clay_Color  background;
	Clay_Color  text;
	Clay_Color  accent;
	Clay_Color  accentText;
	Clay_Color  foreground;
	Clay_Color  highlight;
};

constexpr Clay_Color inline Clay_ColorFromInt(uint32_t value)
{
	return Clay_Color{
		.r = (float)((value >> 16) & 255),
		.g = (float)((value >> 8) & 255),
		.b = (float)((value) & 255),
		.a = (float)(255)
	};
}

constexpr Clay_String Clay_StringFromStdString(const std::string& str)
{
	return { .length = (int32_t)str.length(), .chars = str.c_str() };
};

template <typename T>
class IntervalTimer
{
public:
	IntervalTimer(T interval) : _interval(interval) {}
	bool Update(T dt)
	{
		_time += dt;
		if (_time >= _interval)
		{
			_time = 0;
			return true;
		}
		return false;
	}

private:
	T _time = 0;
	T _interval;
};

class Blinker
{
public:
	Blinker(double interval) : _timer(interval) {};
	void Update(double dt)
	{
		if (_timer.Update(dt))
		{
			State = !State;
		}
	}

	bool State = false;

	operator bool() const { return State; }

private:
	IntervalTimer<double> _timer;
};