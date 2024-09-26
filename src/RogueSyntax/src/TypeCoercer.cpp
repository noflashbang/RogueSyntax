#include "pch.h"

TypeCoercer::TypeCoercer()
{
	_coercionTable[ObjectType::INTEGER_OBJ] = {
		{ ObjectType::INTEGER_OBJ, ObjectType::INTEGER_OBJ },
		{ ObjectType::DECIMAL_OBJ, ObjectType::DECIMAL_OBJ },
		{ ObjectType::BOOLEAN_OBJ, ObjectType::BOOLEAN_OBJ }
	};

	_coercionTable[ObjectType::DECIMAL_OBJ] = {
		{ ObjectType::INTEGER_OBJ, ObjectType::DECIMAL_OBJ },
		{ ObjectType::DECIMAL_OBJ, ObjectType::DECIMAL_OBJ },
		{ ObjectType::BOOLEAN_OBJ, ObjectType::BOOLEAN_OBJ }
	};

	_coercionTable[ObjectType::BOOLEAN_OBJ] = {
		{ ObjectType::INTEGER_OBJ, ObjectType::BOOLEAN_OBJ },
		{ ObjectType::DECIMAL_OBJ, ObjectType::BOOLEAN_OBJ },
		{ ObjectType::BOOLEAN_OBJ, ObjectType::BOOLEAN_OBJ }
	};

	_coercionMap[ObjectType::INTEGER_OBJ] = std::bind(&TypeCoercer::EvalAsInteger, this, std::placeholders::_1);
	_coercionMap[ObjectType::DECIMAL_OBJ] = std::bind(&TypeCoercer::EvalAsDecimal, this, std::placeholders::_1);
	_coercionMap[ObjectType::BOOLEAN_OBJ] = std::bind(&TypeCoercer::EvalAsBoolean, this, std::placeholders::_1);
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
	std::string rName = right->Type().ToString();
	std::string lName = left->Type().ToString();

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
		throw std::runtime_error(std::format("type mismatch can not coerce types: {} -> {}", source->Type().Name, target->Type().Name));
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

	if (obj->Type() == ObjectType::INTEGER_OBJ)
	{
		auto value = dynamic_cast<const IntegerObj const*>(obj)->Value;
		result = value == 0 ? BooleanObj::FALSE_OBJ_REF : BooleanObj::TRUE_OBJ_REF;
	}

	if (obj->Type() == ObjectType::DECIMAL_OBJ)
	{
		auto value = dynamic_cast<const DecimalObj const*>(obj)->Value;
		result = abs(value) <= FLT_EPSILON ? BooleanObj::FALSE_OBJ_REF : BooleanObj::TRUE_OBJ_REF;
	}

	if (result == nullptr)
	{
		throw std::runtime_error(std::format("illegal expression, type {} can not be evaluated as boolean: {}", obj->Type().Name, obj->Inspect()));
	}
		
	return result;
}

std::shared_ptr<IObject> TypeCoercer::EvalAsDecimal(const IObject* const obj) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (obj->Type() != ObjectType::DECIMAL_OBJ)
	{
		if (obj->Type() == ObjectType::INTEGER_OBJ)
		{
			auto value = dynamic_cast<const IntegerObj const*>(obj)->Value;
			result = DecimalObj::New(static_cast<float>(value));
		}

		if (obj->Type() == ObjectType::BOOLEAN_OBJ)
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
		throw std::runtime_error(std::format("illegal expression, type {} can not be evaluated as decimal: {}", obj->Type().Name, obj->Inspect()));
	}

	return result;
}

std::shared_ptr<IObject> TypeCoercer::EvalAsInteger(const IObject* const obj) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (obj->Type() != ObjectType::INTEGER_OBJ)
	{
		if (obj->Type() == ObjectType::DECIMAL_OBJ)
		{
			auto value = dynamic_cast<const DecimalObj const*>(obj)->Value;
			result = IntegerObj::New(static_cast<int>(value));
		}

		if (obj->Type() == ObjectType::BOOLEAN_OBJ)
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
		throw std::runtime_error(std::format("illegal expression, type {} can not be evaluated as integer: {}", obj->Type().Name, obj->Inspect()));
	}

	return result;
}




