#include "pch.h"
#include <iostream>

BuiltIn::BuiltIn(const std::shared_ptr<ObjectFactory> factory) : _factory(factory)
{
	RegisterBuiltIn("len", std::bind(&BuiltIn::Len, this, std::placeholders::_1, std::placeholders::_2));
	RegisterBuiltIn("first", std::bind(&BuiltIn::First, this, std::placeholders::_1, std::placeholders::_2));
	RegisterBuiltIn("last", std::bind(&BuiltIn::Last, this, std::placeholders::_1, std::placeholders::_2));
	RegisterBuiltIn("rest", std::bind(&BuiltIn::Rest, this, std::placeholders::_1, std::placeholders::_2));
	RegisterBuiltIn("push", std::bind(&BuiltIn::Push, this, std::placeholders::_1, std::placeholders::_2));
	RegisterBuiltIn("printLine", std::bind(&BuiltIn::PrintLine, this, std::placeholders::_1, std::placeholders::_2));
}

std::function<IObject* (const std::vector<const IObject*>& args)> BuiltIn::GetBuiltInFunction(const std::string& name)
{
	auto it = std::find(_builtinNames.begin(), _builtinNames.end(), name);
	if (it != _builtinNames.end())
	{
		auto idx = std::distance(_builtinNames.begin(), it);
		auto function = _builtins[idx];
		return Caller(function);
	}
	return nullptr;
}

std::function<IObject* (const std::vector<const IObject*>& args)> BuiltIn::GetBuiltInFunction(const int idx)
{
	if (idx < 0 || idx >= _builtins.size())
	{
		return nullptr;
	}
	auto function = _builtins[idx];
	return Caller(function);
}

std::function<IObject* (const std::vector<const IObject*>& args)> BuiltIn::Caller(std::function<IObject* (const ObjectFactory* factory, const std::vector<const IObject*>& args)> func)
{
	//curry the object factory into the function
	return std::bind(func, _factory.get(), std::placeholders::_1);
}

void BuiltIn::RegisterBuiltIn(const std::string& name, std::function<IObject* (const ObjectFactory* factory, const std::vector<const IObject*>& args)> func)
{
	_builtinNames.push_back(name);
	_builtins.push_back(func);

	//sanity check
	assert((_builtinNames.size() == _builtins.size()));
}

IObject* BuiltIn::Len(const ObjectFactory* factory, const std::vector<const IObject*>& args)
{
	if (args.size() != 1)
	{
		throw std::runtime_error(std::format("wrong number of arguments. got={}, wanted={}", args.size(), 1));
	}

	if (args[0]->IsThisA<StringObj>())
	{
		auto str = dynamic_cast<const StringObj*>(args[0]);
		return factory->New<IntegerObj>(static_cast<int>(str->Value.size()));
	}

	if (args[0]->IsThisA<ArrayObj>())
	{
		auto arr = dynamic_cast<const ArrayObj*>(args[0]);
		return factory->New<IntegerObj>(arr->Elements.size());
	}

	if (args[0]->IsThisA<HashObj>())
	{
		auto hash = dynamic_cast<const HashObj*>(args[0]);
		return factory->New<IntegerObj>(hash->Elements.size());
	}

	throw std::runtime_error(std::format("argument to `len` not supported, got {}", args[0]->TypeName()));
}

IObject* BuiltIn::First(const ObjectFactory* factory, const std::vector<const IObject*>& args)
{
	if (args.size() != 1)
	{
		throw std::runtime_error(std::format("wrong number of arguments. got={}, wanted={}", args.size(), 1));
	}

	if (!args[0]->IsThisA<ArrayObj>())
	{
		throw std::runtime_error(std::format("argument to `first` must be ARRAY, got {}", args[0]->TypeName()));
	}

	auto arr = dynamic_cast<const ArrayObj*>(args[0]);
	if (arr->Elements.size() > 0)
	{
		const IObject* obj = arr->Elements[0];
		return factory->Clone(obj);
	}

	return NullObj::NULL_OBJ_REF;
}

IObject* BuiltIn::Last(const ObjectFactory* factory, const std::vector<const IObject*>& args)
{
	if (args.size() != 1)
	{
		throw std::runtime_error(std::format("wrong number of arguments. got={}, wanted={}", args.size(), 1));
	}

	if (!args[0]->IsThisA<ArrayObj>())
	{
		throw std::runtime_error(std::format("argument to `last` must be ARRAY, got {}", args[0]->TypeName()));
	}

	auto arr = dynamic_cast<const ArrayObj*>(args[0]);
	auto length = arr->Elements.size();
	if (length > 0)
	{
		return factory->Clone(arr->Elements[length - 1]);
	}

	return NullObj::NULL_OBJ_REF;
}

IObject* BuiltIn::Rest(const ObjectFactory* factory, const std::vector<const IObject*>& args)
{
	if (args.size() != 1)
	{
		throw std::runtime_error(std::format("wrong number of arguments. got={}, wanted={}", args.size(), 1));
	}

	if (!args[0]->IsThisA<ArrayObj>())
	{
		throw std::runtime_error(std::format("argument to `rest` must be ARRAY, got {}", args[0]->TypeName()));
	}

	auto arr = dynamic_cast<const ArrayObj*>(args[0]);
	auto length = arr->Elements.size();
	if (length > 0)
	{
		std::vector<const IObject*> newElements;
		for (size_t i = 1; i < length; i++)
		{
			newElements.push_back(factory->Clone(arr->Elements[i]));
		}
		return factory->New<ArrayObj>(newElements);
	}

	return NullObj::NULL_OBJ_REF;
}

IObject* BuiltIn::Push(const ObjectFactory* factory, const std::vector<const IObject*>& args)
{
	if (args.size() != 2)
	{
		throw std::runtime_error(std::format("wrong number of arguments. got={}, wanted={}", args.size(), 2));
	}

	if (!args[0]->IsThisA<ArrayObj>())
	{
		throw std::runtime_error(std::format("argument to `push` must be ARRAY, got {}", args[0]->TypeName()));
	}

	auto arr = dynamic_cast<const ArrayObj*>(args[0]);
	auto newElements = arr->Elements;
	newElements.push_back(args[1]);
	return factory->New<ArrayObj>(newElements);
}

IObject* BuiltIn::PrintLine(const ObjectFactory* factory, const std::vector<const IObject*>& args)
{
	for (const auto& arg : args)
	{
		std::cout << arg->Inspect() << std::endl;
	}
	return VoidObj::VOID_OBJ_REF;
}

