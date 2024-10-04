#include "pch.h"

#include "RecursiveEvaluator.h"
#include "StackEvaluator.h"

Evaluator::Evaluator()
{
	EvalBuiltIn = std::make_shared<BuiltIn>();
	EvalEnvironment = std::make_shared<Environment>();
}

const IObject* Evaluator::Eval(const std::shared_ptr<Program>& program)
{
	uint32_t env = EvalEnvironment->New();
	auto result = Eval(program, env);
	EvalEnvironment->Release(env);
	return result;
}

const IObject* Evaluator::Eval(const std::shared_ptr<Program>& program, const uint32_t env)
{
	const IObject* result = nullptr;
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

const IObject* Evaluator::EvalPrefixExpression(const uint32_t env, const Token& optor, const IObject* right) const
{
	const IObject* result = nullptr;
	if (optor.Type == TokenType::TOKEN_BANG)
	{
		result = EvalBangPrefixOperatorExpression(env, optor, right);
	}
	else if (optor.Type == TokenType::TOKEN_MINUS)
	{
		result = EvalMinusPrefixOperatorExpression(env, optor, right);
	}
	else if (optor.Type == TokenType::TOKEN_BITWISE_NOT)
	{
		result = EvalBitwiseNotPrefixOperatorExpression(env, optor, right);
	}
	else
	{
		result = MakeError(env, std::format("unknown operator: {}", optor.Literal), optor);
	}
	return result;
}

const IObject* Evaluator::EvalInfixExpression(const uint32_t env, const Token& optor, const IObject* left, const IObject* right) const
{
	const IObject* result = nullptr;
	auto store = EvalEnvironment->GetObjectStore(env);
	if (left->IsThisA<NullObj>() || right->IsThisA<NullObj>())
	{
		result = EvalNullInfixExpression(env, optor, left, right);
	}
	else if (left->Type() != right->Type())
	{
		if (_coercer.CanCoerceTypes(left, right))
		{
			auto [left_c, right_c] = _coercer.CoerceTypes(store, left, right);
			
			result = EvalInfixExpression(env, optor, left_c, right_c);
		}
		else
		{
			result = MakeError(env, std::format("type mismatch: {} {} {}", left->TypeName(), optor.Literal, right->TypeName()), optor);
		}
	}
	else if (left->IsThisA<IntegerObj>())
	{
		result = EvalIntegerInfixExpression(env, optor, dynamic_cast<const IntegerObj*>(left), dynamic_cast<const IntegerObj*>(right));
	}
	else if (left->IsThisA<DecimalObj>())
	{
		result = EvalDecimalInfixExpression(env, optor, dynamic_cast<const DecimalObj*>(left), dynamic_cast<const DecimalObj*>(right));
	}
	else if (left->IsThisA<StringObj>())
	{
		result = EvalStringInfixExpression(env, optor, dynamic_cast<const StringObj*>(left), dynamic_cast<const StringObj*>(right));
	}
	else if (left->IsThisA<BooleanObj>())
	{
		result = EvalBooleanInfixExpression(env, optor, dynamic_cast<const BooleanObj*>(left), dynamic_cast<const BooleanObj*>(right));
	}
	else
	{
		result = MakeError(env, std::format("unknown operator: {} {} {}", left->TypeName(), optor.Literal, right->TypeName()), optor);
	}
	return result;
}

const IObject* Evaluator::EvalIndexExpression(const uint32_t env, const Token& op, const IObject* operand, const IObject* index) const
{
	const IObject* result = nullptr;
	if (operand->IsThisA<ArrayObj>() && index->IsThisA<IntegerObj>())
	{
		auto arr = dynamic_cast<const ArrayObj*>(operand);
		auto idx = dynamic_cast<const IntegerObj*>(index);
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
		auto str = dynamic_cast<const StringObj*>(operand);
		auto idx = dynamic_cast<const IntegerObj*>(index);
		if (idx->Value < 0 || idx->Value >= str->Value.size())
		{
			result = NullObj::NULL_OBJ_REF;
		}
		else
		{
			auto store = EvalEnvironment->GetObjectStore(env);
			result = store.New_StringObj(std::string(1, str->Value[idx->Value]));
		}
	}
	else if (operand->IsThisA<HashObj>())
	{
		auto hash = dynamic_cast<const HashObj*>(operand);
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
		result = MakeError(env, std::format("index operator not supported: {}", operand->TypeName()), op);
	}
	return result;
}

const IObject* Evaluator::EvalNullInfixExpression(const uint32_t env, const Token& op, const IObject* const left, const IObject* const right) const
{
	//check what side the null is on
	const IObject* result = nullptr;

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
			auto store = EvalEnvironment->GetObjectStore(env);
			if (nullOnLeft)
			{
				//any other operator on a null and a non-null produces the same non null value
				result = EvalInfixExpression(env, op, store.New_IntegerObj(0), mightBeNullObj->Clone());
			}
			else
			{
				//any other operator on a null and a non-null produces the same non null value
				result = EvalInfixExpression(env, op, mightBeNullObj->Clone(), store.New_IntegerObj(0));
			}
		}
	}
	return result;
}

const IObject* Evaluator::EvalIntegerInfixExpression(const uint32_t env, const Token& optor, const IntegerObj* const left, const IntegerObj* const right) const
{
	const IObject* result = nullptr;
	auto store = EvalEnvironment->GetObjectStore(env);

	if (optor.Type == TokenType::TOKEN_PLUS)
	{
		result = store.New_IntegerObj(left->Value + right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_MINUS)
	{
		result = store.New_IntegerObj(left->Value - right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_ASTERISK)
	{
		result = store.New_IntegerObj(left->Value * right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_SLASH)
	{
		result = store.New_IntegerObj(left->Value / right->Value);
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
		result = store.New_IntegerObj(left->Value & right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_BITWISE_OR)
	{
		result = store.New_IntegerObj(left->Value | right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_BITWISE_XOR)
	{
		result = store.New_IntegerObj(left->Value ^ right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_SHIFT_LEFT)
	{
		result = store.New_IntegerObj(left->Value << right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_SHIFT_RIGHT)
	{
		result = store.New_IntegerObj(left->Value >> right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_MODULO)
	{
		result = store.New_IntegerObj(left->Value % right->Value);
	}
	else
	{
		result = MakeError(env, std::format("unknown operator: {} {} {}", left->TypeName(), optor.Literal, right->TypeName()), optor);
	}
	return result;
}

const IObject* Evaluator::EvalBooleanInfixExpression(const uint32_t env, const Token& optor, const BooleanObj* const left, const BooleanObj* const right) const
{
	const IObject* result = nullptr;
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
		result = MakeError(env, std::format("unknown operator: {} {} {}", left->TypeName(), optor.Literal, right->TypeName()), optor);
	}
	return result;
}

const IObject* Evaluator::EvalDecimalInfixExpression(const uint32_t env, const Token& optor, const DecimalObj* const left, const DecimalObj* const right) const
{
	const IObject* result = nullptr;
	auto store = EvalEnvironment->GetObjectStore(env);

	if (optor.Type == TokenType::TOKEN_PLUS)
	{
		result = store.New_DecimalObj(left->Value + right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_MINUS)
	{
		result = store.New_DecimalObj(left->Value - right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_ASTERISK)
	{
		result = store.New_DecimalObj(left->Value * right->Value);
	}
	else if (optor.Type == TokenType::TOKEN_SLASH)
	{
		result = store.New_DecimalObj(left->Value / right->Value);
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
		result = store.New_DecimalObj(fmod(left->Value, right->Value));
	}
	else
	{
		result = MakeError(env, std::format("unknown operator: {} {} {}", left->TypeName(), optor.Literal, right->TypeName()), optor);
	}
	return result;
}

const IObject* Evaluator::EvalStringInfixExpression(const uint32_t env, const Token& optor, const StringObj* const left, const StringObj* const right) const
{
	const IObject* result = nullptr;
	auto store = EvalEnvironment->GetObjectStore(env);

	if (optor.Type == TokenType::TOKEN_PLUS)
	{
		result = store.New_StringObj(left->Value + right->Value);
	}
	else
	{
		result = MakeError(env, std::format("unknown operator: {} {} {}", left->TypeName(), optor.Literal, right->TypeName()), optor);
	}
	return result;
}

const IObject* Evaluator::EvalAsBoolean(const uint32_t env, const Token& context, const IObject* const obj) const
{
	try
	{
		auto store = EvalEnvironment->GetObjectStore(env);
		return _coercer.EvalAsBoolean(store, obj);
	}
	catch (const std::exception& e)
	{
		return MakeError(env, e.what(), context);
	}
}
const IObject* Evaluator::EvalAsDecimal(const uint32_t env, const Token& context, const IObject* const obj) const
{
	try
	{
		auto store = EvalEnvironment->GetObjectStore(env);
		return _coercer.EvalAsDecimal(store, obj);
	}
	catch (const std::exception& e)
	{
		return MakeError(env, e.what(), context);
	}
}
const IObject* Evaluator::EvalAsInteger(const uint32_t env, const Token& context, const IObject* const obj) const
{
	try
	{
		auto store = EvalEnvironment->GetObjectStore(env);
		return _coercer.EvalAsInteger(store, obj);
	}
	catch (const std::exception& e)
	{
		return MakeError(env, e.what(), context);
	}
}

const IObject* Evaluator::EvalBangPrefixOperatorExpression(const uint32_t env, const Token& optor, const IObject* right) const
{
	const IObject* result = nullptr;
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
		auto value = dynamic_cast<const IntegerObj*>(right)->Value;
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
		result = MakeError(env, std::format("unknown operator: {}{}", optor.Literal, right->TypeName()), optor);
	}
	return result;
}

const IObject* Evaluator::EvalMinusPrefixOperatorExpression(const uint32_t env, const Token& optor, const IObject* right) const
{
	const IObject* result = nullptr;
	auto store = EvalEnvironment->GetObjectStore(env);
	if (!right->IsThisA<IntegerObj>())
	{
		result = MakeError(env, std::format("unknown operator: {}{}", optor.Literal, right->TypeName()), optor);
	}
	else
	{
		auto value = dynamic_cast<const IntegerObj*>(right)->Value;
		result = store.New_IntegerObj(-value);
	}
	return result;
}

const IObject* Evaluator::EvalBitwiseNotPrefixOperatorExpression(const uint32_t env, const Token& optor, const IObject* right) const
{
	const IObject* result = nullptr;
	auto store = EvalEnvironment->GetObjectStore(env);
	if (!right->IsThisA<IntegerObj>())
	{
		result = MakeError(env, std::format("unknown operator: {}{}", optor.Literal, right->TypeName()), optor);
	}
	else
	{
		auto value = dynamic_cast<const IntegerObj*>(right)->Value;
		result = store.New_IntegerObj(~value);
	}
	return result;
}



uint32_t Evaluator::ExtendFunctionEnv(const uint32_t rootEnv, const FunctionObj* func, const std::vector<const IObject*>& args)
{
	auto env = EvalEnvironment->NewEnclosed(rootEnv);
	for (size_t i = 0; i < func->Parameters.size(); i++)
	{
		auto* param = func->Parameters[i];

		if (typeid(*param) != typeid(Identifier))
		{
			continue;
		}

		auto argUnwrapped = UnwrapIfReturnObj(args[i]);
		argUnwrapped = UnwrapIfIdentObj(argUnwrapped);

		auto* ident = dynamic_cast<const Identifier*>(param);
		EvalEnvironment->Set(env, ident->Value, argUnwrapped);
	}
	return env;
}

const IObject* Evaluator::UnwrapIfReturnObj(const IObject* input)
{
	if (input != nullptr && input->IsThisA<ReturnObj>())
	{
		return dynamic_cast<const ReturnObj*>(input)->Value;
	}
	return input;
}

const IObject* Evaluator::UnwrapIfIdentObj(const IObject* input)
{
	if (input != nullptr && input->IsThisA<IdentifierObj>())
	{
		return dynamic_cast<const IdentifierObj*>(input)->Value;
	}
	return input;
}





