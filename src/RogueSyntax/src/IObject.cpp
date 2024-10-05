#include "IObject.h"
#include "pch.h"

//BooleanObj* BooleanObj::TRUE_OBJ_REF = ObjectStore::TRUE_OBJ.get();
//BooleanObj* BooleanObj::FALSE_OBJ_REF = ObjectStore::FALSE_OBJ.get();
//NullObj* NullObj::NULL_OBJ_REF = ObjectStore::NULL_OBJ.get();
//VoidObj* VoidObj::VOID_OBJ_REF = ObjectStore::VOID_OBJ.get();
//ContinueObj* ContinueObj::CONTINUE_OBJ_REF = ObjectStore::CONTINUE_OBJ.get();
//BreakObj* BreakObj::BREAK_OBJ_REF = ObjectStore::BREAK_OBJ.get();

IObject* NullObj::Clone(const ObjectFactory* factory) const
{
	return NULL_OBJ_REF;
}

IObject* VoidObj::Clone(const ObjectFactory* factory) const
{
	return VOID_OBJ_REF;
}

IObject* BooleanObj::Clone(const ObjectFactory* factory) const
{
	return Value ? TRUE_OBJ_REF : FALSE_OBJ_REF;
}

IObject* IntegerObj::Clone(const ObjectFactory* factory) const
{
	return factory->New<IntegerObj>(Value);
}

IObject* DecimalObj::Clone(const ObjectFactory* factory) const
{
	return factory->New<DecimalObj>(Value);
}

IObject* StringObj::Clone(const ObjectFactory* factory) const
{
	return factory->New<StringObj>(Value);
}

IObject* ArrayObj::Clone(const ObjectFactory* factory) const
{
	std::vector<const IObject*> clonedElements;
	std::transform(Elements.begin(), Elements.end(), std::back_inserter(clonedElements), [factory](const auto& elem) { return elem->Clone(factory); });
	return factory->New<ArrayObj>(clonedElements);
}

IObject* HashObj::Clone(const ObjectFactory* factory) const
{
	std::unordered_map<HashKey, HashEntry> clonedElements;
	std::transform(Elements.begin(), Elements.end(), std::inserter(clonedElements, clonedElements.end()), [factory](const auto& elem)
		{
			auto [key, value] = elem;
			return std::make_pair(key, HashEntry{ value.Key->Clone(factory), value.Value->Clone(factory) });
		});
	return factory->New<HashObj>(clonedElements);
}

IObject* FunctionObj::Clone(const ObjectFactory* factory) const
{
	return factory->New<FunctionObj>(Parameters, Body); //shallow copy is fine
}

IObject* BuiltInObj::Clone(const ObjectFactory* factory) const
{
	return Idx != -1 ? factory->New<BuiltInObj>(Idx) : factory->New<BuiltInObj>(Name);
}

IObject* IdentifierObj::Clone(const ObjectFactory* factory) const
{
	return factory->New<IdentifierObj>(Name, Value->Clone(factory));
}

IObject* FunctionCompiledObj::Clone(const ObjectFactory* factory) const
{
	return factory->New<FunctionCompiledObj>(FuncInstructions, NumLocals, NumParameters);
}

IObject* ReturnObj::Clone(const ObjectFactory* factory) const
{
	return factory->New<ReturnObj>(Value->Clone(factory));
}

IObject* ErrorObj::Clone(const ObjectFactory* factory) const
{
	return factory->New<ErrorObj>(Message, Token);
}

IObject* ContinueObj::Clone(const ObjectFactory* factory) const
{
	return CONTINUE_OBJ_REF;
}

IObject* BreakObj::Clone(const ObjectFactory* factory) const
{
	return BREAK_OBJ_REF;
}

IObject* ClosureObj::Clone(const ObjectFactory* factory) const
{
	std::vector<const IObject*> clonedFrees;
	std::transform(Frees.begin(), Frees.end(), std::back_inserter(clonedFrees), [factory](const auto& free) { return free->Clone(factory); });
	return factory->New<ClosureObj>(Function, clonedFrees);
}

const IObject* ArrayObj::Set(const IObject* key, const IObject* value)
{
	if (!key->IsThisA<IntegerObj>())
	{
		throw std::runtime_error("index must be an integer");
	}

	auto index = dynamic_cast<const IntegerObj*>(key);
	if (index->Value < 0 || index->Value >= Elements.size())
	{
		throw std::runtime_error("index out of bounds");
	}

	Elements[index->Value] = value;
	return value;
}

const IObject* HashObj::Set(const IObject* key, const IObject* value)
{
	auto hashKey = HashKey(key->Type(), key->Inspect());
		
	Elements[hashKey] = HashEntry{key, value};
	return value;
}

const IObject* IdentifierObj::Set(const IObject* key, const IObject* value)
{
	Value = value;
	return Value;
}

std::function<IObject*(const std::vector<const IObject*>& args)> BuiltInObj::Resolve(std::shared_ptr<BuiltIn> externals) const
{
	if (Idx != -1)
	{
		return externals->GetBuiltInFunction(Idx);
	}
	return externals->GetBuiltInFunction(Name);
}




