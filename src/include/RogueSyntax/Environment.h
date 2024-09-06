#pragma once

#include "StandardLib.h"

struct Environment
{
	std::unordered_map<std::string, IObject*> Store;
	Environment* Outer;

	Environment();
	Environment(Environment* outer);

	void Set(const std::string& name, IObject* value);
	IObject* Get(const std::string& name);
};