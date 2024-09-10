#pragma once

#include "StandardLib.h"
#include "Token.h"
#include "IObject.h"




class BuiltIn
{
public:

	BuiltIn();
	std::function<std::shared_ptr<IObject>(const std::vector<std::shared_ptr<IObject>>& args, const Token& token)> GetBuiltInFunction(const std::string& name);
	void RegisterBuiltIn(const std::string& name, std::function<std::shared_ptr<IObject>(const std::vector<std::shared_ptr<IObject>>& args, const Token& token)> func);

	bool IsBuiltIn(const std::string& name) const { return _builtins.find(name) != _builtins.end(); }

	static std::shared_ptr<BuiltIn> New() { return std::make_shared<BuiltIn>(); }

	//Built-in functions
	std::shared_ptr<IObject> Len(const std::vector<std::shared_ptr<IObject>>& args, const Token& token);
private:
	std::map<std::string, std::function<std::shared_ptr<IObject>(const std::vector<std::shared_ptr<IObject>>& args, const Token& token)>> _builtins;
};