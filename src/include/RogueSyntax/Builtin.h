#pragma once

#include "StandardLib.h"
#include "Token.h"
#include "IObject.h"

class BuiltIn
{
public:

	BuiltIn();
	std::function<IObject*(const std::vector<const IObject*>& args)> GetBuiltInFunction(const std::string& name);
	std::function<IObject*(const std::vector<const IObject*>& args)> GetBuiltInFunction(const int idx);
	void RegisterBuiltIn(const std::string& name, std::function<IObject* (const std::vector<const IObject*>& args)> func);

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
	IObject* Len(const std::vector<const IObject*>& args);
	IObject* First(const std::vector<const IObject*>& args);
	IObject* Last(const std::vector<const IObject*>& args);
	IObject* Rest(const std::vector<const IObject*>& args);
	IObject* Push(const std::vector<const IObject*>& args);
	IObject* PrintLine(const std::vector<const IObject*>& args);

private:
	std::vector<std::string> _builtinNames;
	std::vector<std::function<IObject* (const std::vector<const IObject*>& args)>> _builtins;
	ObjectStore _objectStore; //hack TODO: fix so that we can use the object store from the environment
};