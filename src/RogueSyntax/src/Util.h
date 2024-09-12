#pragma once

#include "StandardLib.h"

template<typename T, typename B>
T* dynamic_no_copy_cast(const std::shared_ptr<B>& ptr)
{
	return dynamic_cast<T*>(ptr.get());
};