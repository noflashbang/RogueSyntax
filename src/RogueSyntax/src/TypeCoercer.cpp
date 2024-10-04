#include "pch.h"

TypeCoercer::TypeCoercer()
{
	auto intType = typeid(IntegerObj).hash_code();
	auto decType = typeid(DecimalObj).hash_code();
	auto boolType = typeid(BooleanObj).hash_code();
	auto strType = typeid(StringObj).hash_code();
	auto arrType = typeid(ArrayObj).hash_code();


	_coercionTable[intType] = {
		{ decType, decType },
		{ boolType, boolType },
		{ strType, strType }
	};

	_coercionTable[decType] = {
		{ intType, decType },
		{ boolType, boolType },
		{ strType, strType }
	};

	_coercionTable[boolType] = {
		{ intType, boolType },
		{ decType, boolType },
		{ strType, strType }
	};

	_coercionTable[strType] = {
		{ intType, strType },
		{ decType, strType },
		{ boolType, strType },
		{arrType, strType }
	};

	_coercionTable[arrType] = {
		{ strType, strType }
	};

	_coercionMap[intType] = std::bind(&TypeCoercer::EvalAsInteger, this, std::placeholders::_1, std::placeholders::_2);
	_coercionMap[decType] = std::bind(&TypeCoercer::EvalAsDecimal, this, std::placeholders::_1, std::placeholders::_2);
	_coercionMap[boolType] = std::bind(&TypeCoercer::EvalAsBoolean, this, std::placeholders::_1, std::placeholders::_2);
	_coercionMap[strType] = std::bind(&TypeCoercer::EvalAsString, this, std::placeholders::_1, std::placeholders::_2);
}

TypeCoercer::~TypeCoercer()
{
}

bool TypeCoercer::CanCoerceTypes(const IObject* const left, const IObject* const right) const
{
	auto leftCoercion = _coercionTable.find(left->Type());
	if (leftCoercion != _coercionTable.end())
	{
		auto rightCoercion = leftCoercion->second.find(right->Type());
		if (rightCoercion != leftCoercion->second.end())
		{
			return true;
		}
	}
}

std::tuple<IObject*, IObject*> TypeCoercer::CoerceTypes(ObjectStore& store, const IObject* const left, const IObject* const right) const
{
	IObject* leftResult = CoerceThis(store, right, left);
	IObject* rightResult = CoerceThis(store, left, right);

	return std::make_tuple(leftResult, rightResult);
}

IObject* TypeCoercer::CoerceThis(ObjectStore& store, const IObject* const source, const IObject* const target) const
{
	IObject* result = nullptr;

	auto sourceCoercion = _coercionTable.find(source->Type());
	if (sourceCoercion != _coercionTable.end())
	{
		auto targetCoercion = sourceCoercion->second.find(target->Type());
		if (targetCoercion != sourceCoercion->second.end())
		{
			auto rightTT = targetCoercion->second;
			auto coercion = _coercionMap.find(rightTT);
			if (coercion != _coercionMap.end())
			{
				result = coercion->second(store, target);
			}
		}
	}

	if (result == nullptr)
	{
		throw std::runtime_error(std::format("type mismatch can not coerce types: {} -> {}", source->TypeName(), target->TypeName()));
	}
	return result;
}

IObject* TypeCoercer::EvalAsBoolean(ObjectStore& store, const IObject* const obj) const
{
	IObject* result = nullptr;
	if (obj == NullObj::NULL_OBJ_REF)
	{
		result = BooleanObj::FALSE_OBJ_REF;
	}

	if (obj == BooleanObj::TRUE_OBJ_REF)
	{
		result = BooleanObj::TRUE_OBJ_REF;
	}
	
	if(obj == BooleanObj::FALSE_OBJ_REF)
	{
		result = BooleanObj::FALSE_OBJ_REF;
	}

	if (obj->IsThisA<IntegerObj>())
	{
		auto value = dynamic_cast<const IntegerObj const*>(obj)->Value;
		result = value == 0 ? BooleanObj::FALSE_OBJ_REF : BooleanObj::TRUE_OBJ_REF;
	}

	if (obj->IsThisA<DecimalObj>())
	{
		auto value = dynamic_cast<const DecimalObj const*>(obj)->Value;
		result = abs(value) <= FLT_EPSILON ? BooleanObj::FALSE_OBJ_REF : BooleanObj::TRUE_OBJ_REF;
	}

	if (result == nullptr)
	{
		throw std::runtime_error(std::format("illegal expression, type {} can not be evaluated as boolean: {}", obj->TypeName(), obj->Inspect()));
	}
		
	return result;
}

IObject* TypeCoercer::EvalAsDecimal(ObjectStore& store, const IObject* const obj) const
{
	IObject* result = nullptr;
	if (!obj->IsThisA<DecimalObj>())
	{
		if (obj->IsThisA<IntegerObj>())
		{
			auto value = dynamic_cast<const IntegerObj const*>(obj)->Value;
			result = store.New_DecimalObj(static_cast<float>(value));
		}

		if (obj->IsThisA<BooleanObj>())
		{
			auto value = dynamic_cast<const BooleanObj const *>(obj)->Value;
			result = store.New_DecimalObj(value ? 1.0f : 0.0f);
		}
	}
	else
	{
		auto value = dynamic_cast<const DecimalObj const*>(obj)->Value;
		result = store.New_DecimalObj(value);
	}

	if (result == nullptr)
	{
		throw std::runtime_error(std::format("illegal expression, type {} can not be evaluated as decimal: {}", obj->TypeName(), obj->Inspect()));
	}

	return result;
}

IObject* TypeCoercer::EvalAsInteger(ObjectStore& store, const IObject* const obj) const
{
	IObject* result = nullptr;
	if (!obj->IsThisA<IntegerObj>())
	{
		if (obj->IsThisA<DecimalObj>())
		{
			auto value = dynamic_cast<const DecimalObj const*>(obj)->Value;
			result = store.New_IntegerObj(static_cast<int>(value));
		}

		if (obj->IsThisA<BooleanObj>())
		{
			auto value = dynamic_cast<const BooleanObj const*>(obj)->Value;
			result = store.New_IntegerObj(value ? 1 : 0);
		}
	}
	else
	{
		auto value = dynamic_cast<const IntegerObj const*>(obj)->Value;
		result = store.New_IntegerObj(value);
	}

	if (result == nullptr)
	{
		throw std::runtime_error(std::format("illegal expression, type {} can not be evaluated as integer: {}", obj->TypeName(), obj->Inspect()));
	}

	return result;
}

IObject* TypeCoercer::EvalAsString(ObjectStore& store, const IObject* const obj) const
{
	IObject* result = nullptr;
	if (!obj->IsThisA<StringObj>())
	{
		if (obj->IsThisA<IntegerObj>())
		{
			auto value = dynamic_cast<const IntegerObj const*>(obj)->Value;
			result = store.New_StringObj(std::to_string(value));
		}
		else if (obj->IsThisA<DecimalObj>())
		{
			auto value = dynamic_cast<const DecimalObj const*>(obj)->Value;
			result = store.New_StringObj(std::to_string(value));
		}
		else if (obj->IsThisA<BooleanObj>())
		{
			auto value = dynamic_cast<const BooleanObj const*>(obj)->Value;
			result = store.New_StringObj(value ? "true" : "false");
		}
		else if (obj->IsThisA<ArrayObj>())
		{
			auto value = dynamic_cast<const ArrayObj const*>(obj)->Inspect();
			result = store.New_StringObj(value);
		}
	}
	else
	{
		auto value = dynamic_cast<const StringObj const*>(obj)->Value;
		result = store.New_StringObj(value);
	}

	if (result == nullptr)
	{
		throw std::runtime_error(std::format("illegal expression, type {} can not be evaluated as string: {}", obj->TypeName(), obj->Inspect()));
	}

	return result;
}




