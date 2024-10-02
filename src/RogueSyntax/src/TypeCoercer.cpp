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

	_coercionMap[intType] = std::bind(&TypeCoercer::EvalAsInteger, this, std::placeholders::_1);
	_coercionMap[decType] = std::bind(&TypeCoercer::EvalAsDecimal, this, std::placeholders::_1);
	_coercionMap[boolType] = std::bind(&TypeCoercer::EvalAsBoolean, this, std::placeholders::_1);
	_coercionMap[strType] = std::bind(&TypeCoercer::EvalAsString, this, std::placeholders::_1);
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

std::tuple<std::shared_ptr<IObject>, std::shared_ptr<IObject>> TypeCoercer::CoerceTypes(const IObject* const left, const IObject* const right) const
{
	std::shared_ptr<IObject> leftResult = CoerceThis(right, left);
	std::shared_ptr<IObject> rightResult = CoerceThis(left, right);

	return std::make_tuple(leftResult, rightResult);
}

std::shared_ptr<IObject> TypeCoercer::CoerceThis(const IObject* const source, const IObject* const target) const
{
	std::shared_ptr<IObject> result = nullptr;

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
				result = coercion->second(target);
			}
		}
	}

	if (result == nullptr)
	{
		throw std::runtime_error(std::format("type mismatch can not coerce types: {} -> {}", source->TypeName(), target->TypeName()));
	}
	return result;
}

std::shared_ptr<IObject> TypeCoercer::EvalAsBoolean(const IObject* const obj) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (obj == NullObj::NULL_OBJ_REF.get())
	{
		result = BooleanObj::FALSE_OBJ_REF;
	}

	if (obj == BooleanObj::TRUE_OBJ_REF.get())
	{
		result = BooleanObj::TRUE_OBJ_REF;
	}
	
	if(obj == BooleanObj::FALSE_OBJ_REF.get())
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

std::shared_ptr<IObject> TypeCoercer::EvalAsDecimal(const IObject* const obj) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (!obj->IsThisA<DecimalObj>())
	{
		if (obj->IsThisA<IntegerObj>())
		{
			auto value = dynamic_cast<const IntegerObj const*>(obj)->Value;
			result = DecimalObj::New(static_cast<float>(value));
		}

		if (obj->IsThisA<BooleanObj>())
		{
			auto value = dynamic_cast<const BooleanObj const *>(obj)->Value;
			result = DecimalObj::New(value ? 1.0f : 0.0f);
		}
	}
	else
	{
		auto value = dynamic_cast<const DecimalObj const*>(obj)->Value;
		result = DecimalObj::New(value);
	}

	if (result == nullptr)
	{
		throw std::runtime_error(std::format("illegal expression, type {} can not be evaluated as decimal: {}", obj->TypeName(), obj->Inspect()));
	}

	return result;
}

std::shared_ptr<IObject> TypeCoercer::EvalAsInteger(const IObject* const obj) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (!obj->IsThisA<IntegerObj>())
	{
		if (obj->IsThisA<DecimalObj>())
		{
			auto value = dynamic_cast<const DecimalObj const*>(obj)->Value;
			result = IntegerObj::New(static_cast<int>(value));
		}

		if (obj->IsThisA<BooleanObj>())
		{
			auto value = dynamic_cast<const BooleanObj const*>(obj)->Value;
			result = IntegerObj::New(value ? 1 : 0);
		}
	}
	else
	{
		auto value = dynamic_cast<const IntegerObj const*>(obj)->Value;
		result = IntegerObj::New(value);
	}

	if (result == nullptr)
	{
		throw std::runtime_error(std::format("illegal expression, type {} can not be evaluated as integer: {}", obj->TypeName(), obj->Inspect()));
	}

	return result;
}

std::shared_ptr<IObject> TypeCoercer::EvalAsString(const IObject* const obj) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (!obj->IsThisA<StringObj>())
	{
		if (obj->IsThisA<IntegerObj>())
		{
			auto value = dynamic_cast<const IntegerObj const*>(obj)->Value;
			result = StringObj::New(std::to_string(value));
		}
		else if (obj->IsThisA<DecimalObj>())
		{
			auto value = dynamic_cast<const DecimalObj const*>(obj)->Value;
			result = StringObj::New(std::to_string(value));
		}
		else if (obj->IsThisA<BooleanObj>())
		{
			auto value = dynamic_cast<const BooleanObj const*>(obj)->Value;
			result = StringObj::New(value ? "true" : "false");
		}
		else if (obj->IsThisA<ArrayObj>())
		{
			auto value = dynamic_cast<const ArrayObj const*>(obj)->Inspect();
			result = StringObj::New(value);
		}
	}
	else
	{
		auto value = dynamic_cast<const StringObj const*>(obj)->Value;
		result = StringObj::New(value);
	}

	if (result == nullptr)
	{
		throw std::runtime_error(std::format("illegal expression, type {} can not be evaluated as string: {}", obj->TypeName(), obj->Inspect()));
	}

	return result;
}




