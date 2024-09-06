#pragma once

#include "StandardLib.h"

struct Environment
{
	std::unordered_map<std::string, IObject*> Store;
	//RSEnvironment* Outer;

	Environment() = default;

	//Environment(Environment* outer)
	//{
	//	Outer = outer;
	//}

	void Set(const std::string& name, IObject* value);
	IObject* Get(const std::string& name);
};