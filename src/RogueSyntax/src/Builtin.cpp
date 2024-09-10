#include "pch.h"

BuiltIn::BuiltIn()
{
	RegisterBuiltIn("len", std::bind(&BuiltIn::Len, this, std::placeholders::_1, std::placeholders::_2));
}

std::function<std::shared_ptr<IObject>(const std::vector<std::shared_ptr<IObject>>& args, const Token& token)> BuiltIn::GetBuiltInFunction(const std::string& name)
{
	auto it = _builtins.find(name);
	if (it != _builtins.end())
	{
		return it->second;
	}
	return nullptr;
}

void BuiltIn::RegisterBuiltIn(const std::string& name, std::function<std::shared_ptr<IObject>(const std::vector<std::shared_ptr<IObject>>& args, const Token& token)> func)
{
	_builtins[name] = func;
}

std::shared_ptr<IObject> BuiltIn::Len(const std::vector<std::shared_ptr<IObject>>& args, const Token& token)
{
	if (args.size() != 1)
	{
		return ErrorObj::New(std::format("wrong number of arguments. got={}, wanted={}", args.size(), 1), token);
	}

	if (args[0]->Type() == ObjectType::STRING_OBJ)
	{
		auto str = dynamic_cast<StringObj*>(args[0].get());
		return IntegerObj::New(str->Value.size());
	}

	//if (args[0]->Type() == ObjectType::ARRAY_OBJ)
	//{
	//	auto arr = std::dynamic_pointer_cast<ArrayObj>(args[0]);
	//	return IntegerObj::New(arr->Elements.size());
	//}

	return ErrorObj::New(std::format("argument to `len` not supported, got {}", args[0].get()->Type().Name), token);
}