#pragma once

#include "StandardLib.h"

struct Environment
{
	std::unordered_map<std::string, std::shared_ptr<IObject>> Store;
	std::shared_ptr<Environment> Outer;

	Environment();
	Environment(const std::shared_ptr<Environment>& outer);

	void Set(const std::string& name, const std::shared_ptr<IObject>& value);
	std::shared_ptr<IObject> Get(const std::string& name) const;

	static std::shared_ptr<Environment> New();
	static std::shared_ptr<Environment> NewEnclosed(const std::shared_ptr<Environment>& outer);
};