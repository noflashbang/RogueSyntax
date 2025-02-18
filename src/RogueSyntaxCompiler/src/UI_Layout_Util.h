#pragma once

#include <algorithm>

template<typename T>
T SafeClampLow(T value, T low, T high)
{
	if (low > high)
	{
		return low;
	}
	return std::clamp(value, low, high);
};