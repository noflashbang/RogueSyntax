#include "pch.h"

#include "RecursiveEvaluator.h"
#include "StackEvaluator.h"

Evaluator::Evaluator()
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

	_coercionMap[ObjectType::INTEGER_OBJ] = std::bind(&Evaluator::EvalAsInteger, this, std::placeholders::_1, std::placeholders::_2);
	_coercionMap[ObjectType::DECIMAL_OBJ] = std::bind(&Evaluator::EvalAsDecimal, this, std::placeholders::_1, std::placeholders::_2);
	_coercionMap[ObjectType::BOOLEAN_OBJ] = std::bind(&Evaluator::EvalAsBoolean, this, std::placeholders::_1, std::placeholders::_2);

	EvalBuiltIn = std::make_shared<BuiltIn>();
	EvalEnvironment = std::make_shared<Environment>();
}

std::shared_ptr<IObject> Evaluator::Eval(const std::shared_ptr<Program>& program)
{
	uint32_t env = EvalEnvironment->New();
	std::shared_ptr<IObject> result = nullptr;
	for (const auto& stmt : program->Statements)
	{
		result = Eval(stmt, env);
		if (result == nullptr)
		{
			continue;
		}

		if (result->Type() == ObjectType::RETURN_OBJ)
		{
			result = UnwrapIfReturnObj(result);
			
			if (result->Type() == ObjectType::IDENT_OBJ)
			{
				result = UnwrapIfIdentObj(result);
				break;
			}
			break;
		}

		if (result->Type() == ObjectType::IDENT_OBJ)
		{
			result = UnwrapIfIdentObj(result);
			break;
		}

		if (result->Type() == ObjectType::ERROR_OBJ)
		{
			break;
		}
	}
	return result;
}

std::shared_ptr<Evaluator> Evaluator::New(EvaluatorType type)
{
	switch (type)
	{
	case EvaluatorType::Recursive:
		return std::make_shared<RecursiveEvaluator>();
	case EvaluatorType::Stack:
		return std::make_shared<StackEvaluator>();
	default:
		return nullptr;
	}
}

std::shared_ptr<IObject> Evaluator::EvalPrefixExpression(const Token& optor, const std::shared_ptr<IObject>& right) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (optor.Type == TokenType::TOKEN_BANG)
	{
		result = EvalBangPrefixOperatorExpression(optor, right);
	}
	else if (optor.Type == TokenType::TOKEN_MINUS)
	{
		result = EvalMinusPrefixOperatorExpression(optor, right);
	}
	else if (optor.Type == TokenType::TOKEN_BITWISE_NOT)
	{
		result = EvalBitwiseNotPrefixOperatorExpression(optor, right);
	}
	else
	{
		result = MakeError(std::format("unknown operator: {}", optor.Literal), optor);
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalInfixExpression(const Token& optor, const std::shared_ptr<IObject>& left, const std::shared_ptr<IObject>& right) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (left->Type() == ObjectType::NULL_OBJ || right->Type() == ObjectType::NULL_OBJ)
	{
		result = EvalNullInfixExpression(optor, left.get(), right.get());
	}
	else if (left->Type() != right->Type())
	{
		if (CanCoerceTypes(left.get(), right.get()))
		{
			auto [left_c, right_c] = CoerceTypes(optor, left.get(), right.get());
			
			result = EvalInfixExpression(optor, left_c, right_c);
		}
		else
		{
			result = MakeError(std::format("type mismatch: {} {} {}", left->Type().Name, optor.Literal, right->Type().Name), optor);
		}
	}
	else if (left->Type() == ObjectType::INTEGER_OBJ && right->Type() == ObjectType::INTEGER_OBJ)
	{
		result = EvalIntegerInfixExpression(optor, dynamic_cast<IntegerObj*>(left.get()), dynamic_cast<IntegerObj*>(right.get()));
	}
	else if (left->Type() == ObjectType::DECIMAL_OBJ && right->Type() == ObjectType::DECIMAL_OBJ)
	{
		result = EvalDecimalInfixExpression(optor, dynamic_cast<DecimalObj*>(left.get()), dynamic_cast<DecimalObj*>(right.get()));
	}
	else if (left->Type() == ObjectType::STRING_OBJ && right->Type() == ObjectType::STRING_OBJ)
	{
		result = EvalStringInfixExpression(optor, dynamic_cast<StringObj*>(left.get()), dynamic_cast<StringObj*>(right.get()));
	}
	else if (left->Type() == ObjectType::BOOLEAN_OBJ && right->Type() == ObjectType::BOOLEAN_OBJ)
	{
		result = EvalBooleanInfixExpression(optor, dynamic_cast<BooleanObj*>(left.get()), dynamic_cast<BooleanObj*>(right.get()));
	}
	else
	{
		result = MakeError(std::format("unknown operator: {} {} {}", left->Type().Name, optor.Literal, right->Type().Name), optor);
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalIndexExpression(const Token& op, const std::shared_ptr<IObject>& operand, const std::shared_ptr<IObject>& index) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (operand->Type() == ObjectType::ARRAY_OBJ && index->Type() == ObjectType::INTEGER_OBJ)
	{
		auto arr = std::dynamic_pointer_cast<ArrayObj>(operand);
		auto idx = std::dynamic_pointer_cast<IntegerObj>(index);
		if (idx->Value < 0 || idx->Value >= arr->Elements.size())
		{
			result = NullObj::NULL_OBJ_REF;
		}
		else
		{
			result = arr->Elements[idx->Value];
		}
	}
	else if (operand->Type() == ObjectType::STRING_OBJ && index->Type() == ObjectType::INTEGER_OBJ)
	{
		auto str = std::dynamic_pointer_cast<StringObj>(operand);
		auto idx = std::dynamic_pointer_cast<IntegerObj>(index);
		if (idx->Value < 0 || idx->Value >= str->Value.size())
		{
			result = NullObj::NULL_OBJ_REF;
		}
		else
		{
			result = StringObj::New(std::string(1, str->Value[idx->Value]));
		}
	}
	else if (operand->Type() == ObjectType::HASH_OBJ)
	{
		auto hash = std::dynamic_pointer_cast<HashObj>(operand);
		auto key = HashKey { index->Type(), index->Inspect() };

		auto entry = hash->Elements.find(key);
		if (entry != hash->Elements.end())
		{
			result = entry->second.Value;
		}

		if (result == nullptr)
		{
			result = NullObj::NULL_OBJ_REF;
		}
	}
	else
	{
		result = MakeError(std::format("index operator not supported: {}", operand->Type().Name), op);
	}
	return result;

}

bool Evaluator::CanCoerceTypes(const IObject* const left, const IObject* const right) const
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

std::tuple<std::shared_ptr<IObject>, std::shared_ptr<IObject>> Evaluator::CoerceTypes(const Token& context, const IObject* const left, const IObject* const right) const
{
	std::string rName = right->Type().ToString();
	std::string lName = left->Type().ToString();

	std::shared_ptr<IObject> leftResult = CoerceThis(context, right, left);
	std::shared_ptr<IObject> rightResult = CoerceThis(context, left, right);

	return std::make_tuple(leftResult, rightResult);
}

std::shared_ptr<IObject> Evaluator::CoerceThis(const Token& context, const IObject* const source, const IObject* const target) const
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
				result = coercion->second(context, target);
			}
		}
	}

	if (result == nullptr)
	{
		result = MakeError(std::format("type mismatch can not coerce types: {} -> {}", source->Type().Name, target->Type().Name), context);
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalAsBoolean(const Token& context, const IObject* const obj) const
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
		result = MakeError(std::format("illegal expression, type {} can not be evaluated as boolean: {}", obj->Type().Name, obj->Inspect()), context);
	}
		
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalAsDecimal(const Token& context, const IObject* const obj) const
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
		result = MakeError(std::format("illegal expression, type {} can not be evaluated as decimal: {}", obj->Type().Name, obj->Inspect()), context);
	}

	return result;
}

std::shared_ptr<IObject> Evaluator::EvalAsInteger(const Token& context, const IObject* const obj) const
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
		result = MakeError(std::format("illegal expression, type {} can not be evaluated as integer: {}", obj->Type().Name, obj->Inspect()), context);
	}

	return result;
}

std::shared_ptr<IObject> Evaluator::EvalNullInfixExpression(const Token& op, const IObject* const left, const IObject* const right) const
{
	//check what side the null is on
	std::shared_ptr<IObject> result = nullptr;

	const IObject* nullObj = nullptr;
	const IObject* mightBeNullObj = nullptr;

	bool nullOnLeft = false;
	if (left->Type() == ObjectType::NULL_OBJ)
	{
		nullObj = left;
		mightBeNullObj = right;
		nullOnLeft = true;
	}
	else
	{
		nullObj = right;
		mightBeNullObj = left;
		nullOnLeft = false;
	}
	
	if (mightBeNullObj->Type() == ObjectType::NULL_OBJ)
	{
		if (op.Type == TokenType::TOKEN_EQ)
		{
			result = BooleanObj::TRUE_OBJ_REF;
		}
		else if (op.Type == TokenType::TOKEN_NOT_EQ)
		{
			result = BooleanObj::FALSE_OBJ_REF;
		}
		else
		{
			//any other operator on two nulls produces a null
			result = NullObj::NULL_OBJ_REF;
		}
	}
	else
	{
		//check for equality
		if (op.Type == TokenType::TOKEN_EQ)
		{
			result = BooleanObj::FALSE_OBJ_REF;
		}
		else if (op.Type == TokenType::TOKEN_NOT_EQ)
		{
			result = BooleanObj::TRUE_OBJ_REF;
		}
		else
		{
			if (nullOnLeft)
			{
				//any other operator on a null and a non-null produces the same non null value
				result = EvalInfixExpression(op, IntegerObj::New(0), mightBeNullObj->Clone());
			}
			else
			{
				//any other operator on a null and a non-null produces the same non null value
				result = EvalInfixExpression(op, mightBeNullObj->Clone(), IntegerObj::New(0));
			}
		}
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalIntegerInfixExpression(const Token& optor, const IntegerObj* const left, const IntegerObj* const right) const
{
	std::shared_ptr<IObject> result = nullptr;

	if (optor.Type == TokenType::TOKEN_PLUS)
	{
		result = IntegerObj::New(left->Value + right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_MINUS)
	{
		result = IntegerObj::New(left->Value - right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_ASTERISK)
	{
		result = IntegerObj::New(left->Value * right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_SLASH)
	{
		result = IntegerObj::New(left->Value / right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_LT)
	{
		result = left->Value < right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_GT)
	{
		result = left->Value > right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_GT_EQ)
	{
		result = left->Value >= right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_LT_EQ)
	{
		result = left->Value <= right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_EQ)
	{
		result = left->Value == right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_NOT_EQ)
	{
		result = left->Value != right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_BITWISE_AND)
	{
		result = IntegerObj::New(left->Value & right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_BITWISE_OR)
	{
		result = IntegerObj::New(left->Value | right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_BITWISE_XOR)
	{
		result = IntegerObj::New(left->Value ^ right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_SHIFT_LEFT)
	{
		result = IntegerObj::New(left->Value << right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_SHIFT_RIGHT)
	{
		result = IntegerObj::New(left->Value >> right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_MODULO)
	{
		result = IntegerObj::New(left->Value % right->Value);
	}
	else
	{
		result = MakeError(std::format("unknown operator: {} {} {}", left->Type().Name, optor.Literal, right->Type().Name), optor);
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalBooleanInfixExpression(const Token& optor, const BooleanObj* const left, const BooleanObj* const right) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (optor.Type == TokenType::TOKEN_EQ)
	{
		result = left->Value == right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_NOT_EQ)
	{
		result = left->Value != right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_AND)
	{
		result = left->Value && right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_OR)
	{
		result = left->Value || right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else
	{
		result = MakeError(std::format("unknown operator: {} {} {}", left->Type().Name, optor.Literal, right->Type().Name), optor);
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalDecimalInfixExpression(const Token& optor, const DecimalObj* const left, const DecimalObj* const right) const
{
	std::shared_ptr<IObject> result = nullptr;

	if (optor.Type == TokenType::TOKEN_PLUS)
	{
		result = DecimalObj::New(left->Value + right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_MINUS)
	{
		result = DecimalObj::New(left->Value - right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_ASTERISK)
	{
		result = DecimalObj::New(left->Value * right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_SLASH)
	{
		result = DecimalObj::New(left->Value / right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_LT)
	{
		result = left->Value < right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_GT)
	{
		result = left->Value > right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_GT_EQ)
	{
		result = left->Value >= right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_LT_EQ)
	{
		result = left->Value <= right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_EQ)
	{
		result = abs(left->Value - right->Value) <= FLT_EPSILON ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_NOT_EQ)
	{
		result = abs(left->Value - right->Value) > FLT_EPSILON ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_MODULO)
	{
		result = DecimalObj::New(fmod(left->Value, right->Value));
	}
	else
	{
		result = MakeError(std::format("unknown operator: {} {} {}", left->Type().Name, optor.Literal, right->Type().Name), optor);
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalStringInfixExpression(const Token& optor, const StringObj* const left, const StringObj* const right) const
{
	std::shared_ptr<IObject> result = nullptr;

	if (optor.Type == TokenType::TOKEN_PLUS)
	{
		result = StringObj::New(left->Value + right->Value);
	}
	else
	{
		result = MakeError(std::format("unknown operator: {} {} {}", left->Type().Name, optor.Literal, right->Type().Name), optor);
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalBangPrefixOperatorExpression(const Token& optor, const std::shared_ptr<IObject>& right) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (right == BooleanObj::TRUE_OBJ_REF)
	{
		result = BooleanObj::FALSE_OBJ_REF;
	}
	else if (right == BooleanObj::FALSE_OBJ_REF || right == NullObj::NULL_OBJ_REF)
	{
		result = BooleanObj::TRUE_OBJ_REF;
	}
	else if (right->Type() == ObjectType::INTEGER_OBJ)
	{
		auto value = dynamic_cast<IntegerObj*>(right.get())->Value;
		if (value == 0)
		{
			result = BooleanObj::TRUE_OBJ_REF;
		}
		else
		{
			result = BooleanObj::FALSE_OBJ_REF;
		}
	}
	else
	{
		result = MakeError(std::format("unknown operator: {}{}", optor.Literal, right->Type().Name), optor);
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalMinusPrefixOperatorExpression(const Token& optor, const std::shared_ptr<IObject>& right) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (right->Type() != ObjectType::INTEGER_OBJ)
	{
		result = MakeError(std::format("unknown operator: {}{}", optor.Literal, right->Type().Name), optor);
	}
	else
	{
		auto value = dynamic_cast<IntegerObj*>(right.get())->Value;
		result = IntegerObj::New(-value);
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalBitwiseNotPrefixOperatorExpression(const Token& optor, const std::shared_ptr<IObject>& right) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (right->Type() != ObjectType::INTEGER_OBJ)
	{
		result = MakeError(std::format("unknown operator: {}{}", optor.Literal, right->Type().Name), optor);
	}
	else
	{
		auto value = dynamic_cast<IntegerObj*>(right.get())->Value;
		result = IntegerObj::New(~value);
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::MakeError(const std::string& message, const Token& token)
{
	return ErrorObj::New(message, token);
}

uint32_t Evaluator::ExtendFunctionEnv(const uint32_t rootEnv, const std::shared_ptr<FunctionObj>& func, const std::vector<std::shared_ptr<IObject>>& args)
{
	auto env = EvalEnvironment->NewEnclosed(rootEnv);
	for (size_t i = 0; i < func->Parameters.size(); i++)
	{
		auto* param = func->Parameters[i].get();

		if (param->NType() != NodeType::Identifier)
		{
			continue;
		}

		auto argUnwrapped = UnwrapIfReturnObj(args[i]);
		argUnwrapped = UnwrapIfIdentObj(argUnwrapped);

		auto* ident = dynamic_cast<Identifier*>(param);
		EvalEnvironment->Set(env, ident->Value, argUnwrapped);
	}
	return env;
}

std::shared_ptr<IObject> Evaluator::UnwrapIfReturnObj(const std::shared_ptr<IObject>& input)
{
	if (input != nullptr && input->Type() == ObjectType::RETURN_OBJ)
	{
		return dynamic_cast<ReturnObj*>(input.get())->Value;
	}
	return input;
}

std::shared_ptr<IObject> Evaluator::UnwrapIfIdentObj(const std::shared_ptr<IObject>& input)
{
	if (input != nullptr && input->Type() == ObjectType::IDENT_OBJ)
	{
		return dynamic_cast<IdentifierObj*>(input.get())->Value;
	}
	return input;
}





