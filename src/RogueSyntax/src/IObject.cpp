#include "IObject.h"
#include "pch.h"

BooleanObj BooleanObj::TRUE_OBJ = BooleanObj(true);
BooleanObj BooleanObj::FALSE_OBJ = BooleanObj(false);
BooleanObj* BooleanObj::TRUE_OBJ_REF = &BooleanObj::TRUE_OBJ;
BooleanObj* BooleanObj::FALSE_OBJ_REF = &BooleanObj::FALSE_OBJ;

NullObj NullObj::NULL_OBJ = NullObj();
NullObj* NullObj::NULL_OBJ_REF = &NullObj::NULL_OBJ;

VoidObj VoidObj::VOID_OBJ = VoidObj();
VoidObj* VoidObj::VOID_OBJ_REF = &VoidObj::VOID_OBJ;

ContinueObj ContinueObj::CONTINUE_OBJ = ContinueObj();
ContinueObj* ContinueObj::CONTINUE_OBJ_REF = &ContinueObj::CONTINUE_OBJ;

BreakObj BreakObj::BREAK_OBJ = BreakObj();
BreakObj* BreakObj::BREAK_OBJ_REF = &BreakObj::BREAK_OBJ;

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




