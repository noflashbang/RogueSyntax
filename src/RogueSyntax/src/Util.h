#pragma once

#include "StandardLib.h"
#include "IObject.h"

template<typename T, typename B>
T* dynamic_no_copy_cast(const std::shared_ptr<B>& ptr)
{
	return dynamic_cast<T*>(ptr.get());
};

static inline std::shared_ptr<IObject> MakeError(const std::string& message, const Token& token)
{
	return ErrorObj::New(message, token);
};