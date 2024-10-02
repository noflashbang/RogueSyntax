#include "pch.h"

#include "RecursiveEvaluator.h"
#include "StackEvaluator.h"

Evaluator::Evaluator()
{
	EvalBuiltIn = std::make_shared<BuiltIn>();
	EvalEnvironment = std::make_shared<Environment>();
}

std::shared_ptr<IObject> Evaluator::Eval(const std::shared_ptr<Program>& program)
{
	uint32_t env = EvalEnvironment->New();
	auto result = Eval(program, env);
	EvalEnvironment->Release(env);
	return result;
}

std::shared_ptr<IObject> Evaluator::Eval(const std::shared_ptr<Program>& program, const uint32_t env)
{
	std::shared_ptr<IObject> result = nullptr;
	for (const auto& stmt : program->Statements)
	{
		result = Eval(stmt, env);
		if (result == nullptr)
		{
			continue;
		}

		if (result->IsThisA<ReturnObj>())
		{
			result = UnwrapIfReturnObj(result);

			if (result->IsThisA<IdentifierObj>())
			{
				result = UnwrapIfIdentObj(result);
				break;
			}
			break;
		}

		if (result->IsThisA<IdentifierObj>())
		{
			result = UnwrapIfIdentObj(result);
			break;
		}

		if (result->IsThisA<ErrorObj>())
		{
			break;
		}
	}
	return result;
}

uint32_t Evaluator::MakeEnv()
{
	return EvalEnvironment->New();

}

void Evaluator::FreeEnv(const uint32_t env)
{
	EvalEnvironment->Release(env);
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
	if (left->IsThisA<NullObj>() || right->IsThisA<NullObj>())
	{
		result = EvalNullInfixExpression(optor, left.get(), right.get());
	}
	else if (left->Type() != right->Type())
	{
		if (_coercer.CanCoerceTypes(left.get(), right.get()))
		{
			auto [left_c, right_c] = _coercer.CoerceTypes(left.get(), right.get());
			
			result = EvalInfixExpression(optor, left_c, right_c);
		}
		else
		{
			result = MakeError(std::format("type mismatch: {} {} {}", left->TypeName(), optor.Literal, right->TypeName()), optor);
		}
	}
	else if (left->IsThisA<IntegerObj>())
	{
		result = EvalIntegerInfixExpression(optor, dynamic_cast<IntegerObj*>(left.get()), dynamic_cast<IntegerObj*>(right.get()));
	}
	else if (left->IsThisA<DecimalObj>())
	{
		result = EvalDecimalInfixExpression(optor, dynamic_cast<DecimalObj*>(left.get()), dynamic_cast<DecimalObj*>(right.get()));
	}
	else if (left->IsThisA<StringObj>())
	{
		result = EvalStringInfixExpression(optor, dynamic_cast<StringObj*>(left.get()), dynamic_cast<StringObj*>(right.get()));
	}
	else if (left->IsThisA<BooleanObj>())
	{
		result = EvalBooleanInfixExpression(optor, dynamic_cast<BooleanObj*>(left.get()), dynamic_cast<BooleanObj*>(right.get()));
	}
	else
	{
		result = MakeError(std::format("unknown operator: {} {} {}", left->TypeName(), optor.Literal, right->TypeName()), optor);
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalIndexExpression(const Token& op, const std::shared_ptr<IObject>& operand, const std::shared_ptr<IObject>& index) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (operand->IsThisA<ArrayObj>() && index->IsThisA<IntegerObj>())
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
	else if (operand->IsThisA<StringObj>() && index->IsThisA<IntegerObj>())
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
	else if (operand->IsThisA<HashObj>())
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
		result = MakeError(std::format("index operator not supported: {}", operand->TypeName()), op);
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
	if (left->IsThisA<NullObj>())
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
	
	if (mightBeNullObj->IsThisA<NullObj>())
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
		result = MakeError(std::format("unknown operator: {} {} {}", left->TypeName(), optor.Literal, right->TypeName()), optor);
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
		result = MakeError(std::format("unknown operator: {} {} {}", left->TypeName(), optor.Literal, right->TypeName()), optor);
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
		result = MakeError(std::format("unknown operator: {} {} {}", left->TypeName(), optor.Literal, right->TypeName()), optor);
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
		result = MakeError(std::format("unknown operator: {} {} {}", left->TypeName(), optor.Literal, right->TypeName()), optor);
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalAsBoolean(const Token& context, const IObject* const obj) const
{
	try
	{
		return _coercer.EvalAsBoolean(obj);
	}
	catch (const std::exception& e)
	{
		return MakeError(e.what(), context);
	}
}
std::shared_ptr<IObject> Evaluator::EvalAsDecimal(const Token& context, const IObject* const obj) const
{
	try
	{
		return _coercer.EvalAsDecimal(obj);
	}
	catch (const std::exception& e)
	{
		return MakeError(e.what(), context);
	}
}
std::shared_ptr<IObject> Evaluator::EvalAsInteger(const Token& context, const IObject* const obj) const
{
	try
	{
		return _coercer.EvalAsInteger(obj);
	}
	catch (const std::exception& e)
	{
		return MakeError(e.what(), context);
	}
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
	else if (right->IsThisA<IntegerObj>())
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
		result = MakeError(std::format("unknown operator: {}{}", optor.Literal, right->TypeName()), optor);
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalMinusPrefixOperatorExpression(const Token& optor, const std::shared_ptr<IObject>& right) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (!right->IsThisA<IntegerObj>())
	{
		result = MakeError(std::format("unknown operator: {}{}", optor.Literal, right->TypeName()), optor);
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
	if (!right->IsThisA<IntegerObj>())
	{
		result = MakeError(std::format("unknown operator: {}{}", optor.Literal, right->TypeName()), optor);
	}
	else
	{
		auto value = dynamic_cast<IntegerObj*>(right.get())->Value;
		result = IntegerObj::New(~value);
	}
	return result;
}



uint32_t Evaluator::ExtendFunctionEnv(const uint32_t rootEnv, const std::shared_ptr<FunctionObj>& func, const std::vector<std::shared_ptr<IObject>>& args)
{
	auto env = EvalEnvironment->NewEnclosed(rootEnv);
	for (size_t i = 0; i < func->Parameters.size(); i++)
	{
		auto* param = func->Parameters[i].get();

		if (typeid(*param) != typeid(Identifier))
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
	if (input != nullptr && input->IsThisA<ReturnObj>())
	{
		return dynamic_cast<ReturnObj*>(input.get())->Value;
	}
	return input;
}

std::shared_ptr<IObject> Evaluator::UnwrapIfIdentObj(const std::shared_ptr<IObject>& input)
{
	if (input != nullptr && input->IsThisA<IdentifierObj>())
	{
		return dynamic_cast<IdentifierObj*>(input.get())->Value;
	}
	return input;
}





