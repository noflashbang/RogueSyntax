#pragma once

#include "StandardLib.h"
#include "Token.h"
#include "IObject.h"

class BuiltIn
{
public:

	BuiltIn(const std::shared_ptr<ObjectFactory> factory);
	std::function<IObject*(const std::vector<const IObject*>& args)> GetBuiltInFunction(const std::string& name);
	std::function<IObject*(const std::vector<const IObject*>& args)> GetBuiltInFunction(const int idx);
	void RegisterBuiltIn(const std::string& name, std::function<IObject* (const ObjectFactory* factory, const std::vector<const IObject*>& args)> func);

	std::function<IObject* (const std::vector<const IObject*>& args)> Caller(std::function<IObject* (const ObjectFactory* factory, const std::vector<const IObject*>& args)> func);

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
	//Built-in functions
	IObject* Len(const ObjectFactory* factory, const std::vector<const IObject*>& args);
	IObject* First(const ObjectFactory* factory, const std::vector<const IObject*>& args);
	IObject* Last(const ObjectFactory* factory, const std::vector<const IObject*>& args);
	IObject* Rest(const ObjectFactory* factory, const std::vector<const IObject*>& args);
	IObject* Push(const ObjectFactory* factory, const std::vector<const IObject*>& args);
	IObject* PrintLine(const ObjectFactory* factory, const std::vector<const IObject*>& args);

private:
	std::vector<std::string> _builtinNames;
	std::vector<std::function<IObject* (const ObjectFactory* factory, const std::vector<const IObject*>& args)>> _builtins;
	std::shared_ptr<ObjectFactory> _factory;
};