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

std::function<IObject* (const std::vector<const IObject*>& args)> BuiltIn::GetBuiltInFunction(const std::string& name)
{
	auto it = std::find(_builtinNames.begin(), _builtinNames.end(), name);
	if (it != _builtinNames.end())
	{
		auto idx = std::distance(_builtinNames.begin(), it);
		return _builtins[idx];
	}
	return nullptr;
}

std::function<IObject* (const std::vector<const IObject*>& args)> BuiltIn::GetBuiltInFunction(const int idx)
{
	if (idx < 0 || idx >= _builtins.size())
	{
		return nullptr;
	}
	return _builtins[idx];
}

void BuiltIn::RegisterBuiltIn(const std::string& name, std::function<IObject* (const std::vector<const IObject*>& args)> func)
{
	_builtinNames.push_back(name);
	_builtins.push_back(func);

	//sanity check
	_ASSERT(_builtinNames.size() == _builtins.size());
}

IObject* BuiltIn::Len(const std::vector<const IObject*>& args)
{
	if (args.size() != 1)
	{
		throw std::runtime_error(std::format("wrong number of arguments. got={}, wanted={}", args.size(), 1));
	}

	if (args[0]->IsThisA<StringObj>())
	{
		auto str = dynamic_cast<const StringObj*>(args[0]);
		return _objectStore.New_IntegerObj(str->Value.size());
	}

	if (args[0]->IsThisA<ArrayObj>())
	{
		auto arr = dynamic_cast<const ArrayObj*>(args[0]);
		return _objectStore.New_IntegerObj(arr->Elements.size());
	}

	if (args[0]->IsThisA<HashObj>())
	{
		auto hash = dynamic_cast<const HashObj*>(args[0]);
		return _objectStore.New_IntegerObj(hash->Elements.size());
	}

	throw std::runtime_error(std::format("argument to `len` not supported, got {}", args[0]->TypeName()));
}

IObject* BuiltIn::First(const std::vector<const IObject*>& args)
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
		auto cloned = arr->Elements[0]->Clone();
		_objectStore.Add(cloned);
		return cloned;
	}

	return NullObj::NULL_OBJ_REF;
}

IObject* BuiltIn::Last(const std::vector<const IObject*>& args)
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
		auto cloned = arr->Elements[length - 1]->Clone();
		_objectStore.Add(cloned);
		return cloned;
	}

	return NullObj::NULL_OBJ_REF;
}

IObject* BuiltIn::Rest(const std::vector<const IObject*>& args)
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
			newElements.push_back(arr->Elements[i]);
		}
		return _objectStore.New_ArrayObj(newElements);
	}

	return NullObj::NULL_OBJ_REF;
}

IObject* BuiltIn::Push(const std::vector<const IObject*>& args)
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
	return _objectStore.New_ArrayObj(newElements);
}

IObject* BuiltIn::PrintLine(const std::vector<const IObject*>& args)
{
	for (const auto& arg : args)
	{
		std::cout << arg->Inspect() << std::endl;
	}
	return VoidObj::VOID_OBJ_REF;
}

