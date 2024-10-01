#include "pch.h"
#include <iostream>

BuiltIn::BuiltIn()
{
	RegisterBuiltIn("len", std::bind(&BuiltIn::Len, this, std::placeholders::_1));
	RegisterBuiltIn("first", std::bind(&BuiltIn::First, this, std::placeholders::_1));
	RegisterBuiltIn("last", std::bind(&BuiltIn::Last, this, std::placeholders::_1));
	RegisterBuiltIn("rest", std::bind(&BuiltIn::Rest, this, std::placeholders::_1));
	RegisterBuiltIn("push", std::bind(&BuiltIn::Push, this, std::placeholders::_1));
	RegisterBuiltIn("printLine", std::bind(&BuiltIn::PrintLine, this, std::placeholders::_1));
}

std::function<std::shared_ptr<IObject>(const std::vector<std::shared_ptr<IObject>>& args)> BuiltIn::GetBuiltInFunction(const std::string& name)
{
	auto it = std::find(_builtinNames.begin(), _builtinNames.end(), name);
	if (it != _builtinNames.end())
	{
		auto idx = std::distance(_builtinNames.begin(), it);
		return _builtins[idx];
	}
	return nullptr;
}

std::function<std::shared_ptr<IObject>(const std::vector<std::shared_ptr<IObject>>& args)> BuiltIn::GetBuiltInFunction(const int idx)
{
	if (idx < 0 || idx >= _builtins.size())
	{
		return nullptr;
	}
	return _builtins[idx];
}

void BuiltIn::RegisterBuiltIn(const std::string& name, std::function<std::shared_ptr<IObject>(const std::vector<std::shared_ptr<IObject>>& args)> func)
{
	_builtinNames.push_back(name);
	_builtins.push_back(func);

	//sanity check
	_ASSERT(_builtinNames.size() == _builtins.size());
}

std::shared_ptr<IObject> BuiltIn::Len(const std::vector<std::shared_ptr<IObject>>& args)
{
	if (args.size() != 1)
	{
		throw std::runtime_error(std::format("wrong number of arguments. got={}, wanted={}", args.size(), 1));
	}

	if (args[0]->Type() == ObjectType::STRING_OBJ)
	{
		auto str = dynamic_cast<StringObj*>(args[0].get());
		return IntegerObj::New(str->Value.size());
	}

	if (args[0]->Type() == ObjectType::ARRAY_OBJ)
	{
		auto arr = std::dynamic_pointer_cast<ArrayObj>(args[0]);
		return IntegerObj::New(arr->Elements.size());
	}

	if (args[0]->Type() == ObjectType::HASH_OBJ)
	{
		auto hash = std::dynamic_pointer_cast<HashObj>(args[0]);
		return IntegerObj::New(hash->Elements.size());
	}

	throw std::runtime_error(std::format("argument to `len` not supported, got {}", args[0].get()->Type().Name));
}

std::shared_ptr<IObject> BuiltIn::First(const std::vector<std::shared_ptr<IObject>>& args)
{
	if (args.size() != 1)
	{
		throw std::runtime_error(std::format("wrong number of arguments. got={}, wanted={}", args.size(), 1));
	}

	if (args[0]->Type() != ObjectType::ARRAY_OBJ)
	{
		throw std::runtime_error(std::format("argument to `first` must be ARRAY, got {}", args[0].get()->Type().Name));
	}

	auto arr = std::dynamic_pointer_cast<ArrayObj>(args[0]);
	if (arr->Elements.size() > 0)
	{
		return arr->Elements[0];
	}

	return NullObj::NULL_OBJ_REF;
}

std::shared_ptr<IObject> BuiltIn::Last(const std::vector<std::shared_ptr<IObject>>& args)
{
	if (args.size() != 1)
	{
		throw std::runtime_error(std::format("wrong number of arguments. got={}, wanted={}", args.size(), 1));
	}

	if (args[0]->Type() != ObjectType::ARRAY_OBJ)
	{
		throw std::runtime_error(std::format("argument to `last` must be ARRAY, got {}", args[0].get()->Type().Name));
	}

	auto arr = std::dynamic_pointer_cast<ArrayObj>(args[0]);
	auto length = arr->Elements.size();
	if (length > 0)
	{
		return arr->Elements[length - 1];
	}

	return NullObj::NULL_OBJ_REF;
}

std::shared_ptr<IObject> BuiltIn::Rest(const std::vector<std::shared_ptr<IObject>>& args)
{
	if (args.size() != 1)
	{
		throw std::runtime_error(std::format("wrong number of arguments. got={}, wanted={}", args.size(), 1));
	}

	if (args[0]->Type() != ObjectType::ARRAY_OBJ)
	{
		throw std::runtime_error(std::format("argument to `rest` must be ARRAY, got {}", args[0].get()->Type().Name));
	}

	auto arr = std::dynamic_pointer_cast<ArrayObj>(args[0]);
	auto length = arr->Elements.size();
	if (length > 0)
	{
		std::vector<std::shared_ptr<IObject>> newElements;
		for (size_t i = 1; i < length; i++)
		{
			newElements.push_back(arr->Elements[i]);
		}
		return ArrayObj::New(newElements);
	}

	return NullObj::NULL_OBJ_REF;
}

std::shared_ptr<IObject> BuiltIn::Push(const std::vector<std::shared_ptr<IObject>>& args)
{
	if (args.size() != 2)
	{
		throw std::runtime_error(std::format("wrong number of arguments. got={}, wanted={}", args.size(), 2));
	}

	if (args[0]->Type() != ObjectType::ARRAY_OBJ)
	{
		throw std::runtime_error(std::format("argument to `push` must be ARRAY, got {}", args[0].get()->Type().Name));
	}

	auto arr = std::dynamic_pointer_cast<ArrayObj>(args[0]);
	auto newElements = arr->Elements;
	newElements.push_back(args[1]);
	return ArrayObj::New(newElements);
}

std::shared_ptr<IObject> BuiltIn::PrintLine(const std::vector<std::shared_ptr<IObject>>& args)
{
	for (const auto& arg : args)
	{
		std::cout << arg->Inspect() << std::endl;
	}
	return VoidObj::VOID_OBJ_REF;
}

