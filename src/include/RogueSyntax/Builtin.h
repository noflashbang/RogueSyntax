#pragma once

#include "StandardLib.h"
#include "Token.h"
#include "IObject.h"

class BuiltIn
{
public:

	BuiltIn();
	std::function<std::shared_ptr<IObject>(const std::vector<std::shared_ptr<IObject>>& args)> GetBuiltInFunction(const std::string& name);
	std::function<std::shared_ptr<IObject>(const std::vector<std::shared_ptr<IObject>>& args)> GetBuiltInFunction(const int idx);
	void RegisterBuiltIn(const std::string& name, std::function<std::shared_ptr<IObject>(const std::vector<std::shared_ptr<IObject>>& args)> func);

	bool IsBuiltIn(const std::string& name) const { return  std::find(_builtinNames.begin(), _builtinNames.end(), name) != _builtinNames.end(); }
	int BuiltInIdx(const std::string& name) const 
	{
		auto it = std::find(_builtinNames.begin(), _builtinNames.end(), name);
		if (it != _builtinNames.end())
		{
			return std::distance(_builtinNames.begin(), it);
		}
		return -1;
	}

	static std::shared_ptr<BuiltIn> New() { return std::make_shared<BuiltIn>(); }

	//Built-in functions
	std::shared_ptr<IObject> Len(const std::vector<std::shared_ptr<IObject>>& args);
	std::shared_ptr<IObject> First(const std::vector<std::shared_ptr<IObject>>& args);
	std::shared_ptr<IObject> Last(const std::vector<std::shared_ptr<IObject>>& args);
	std::shared_ptr<IObject> Rest(const std::vector<std::shared_ptr<IObject>>& args);
	std::shared_ptr<IObject> Push(const std::vector<std::shared_ptr<IObject>>& args);
	std::shared_ptr<IObject> PrintLine(const std::vector<std::shared_ptr<IObject>>& args);

private:
	std::vector<std::string> _builtinNames;
	std::vector<std::function<std::shared_ptr<IObject>(const std::vector<std::shared_ptr<IObject>>& args)>> _builtins;
};