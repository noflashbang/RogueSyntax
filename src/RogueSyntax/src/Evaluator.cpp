#include "Evaluator.h"
#include "Evaluator.h"
#include "Evaluator.h"
#include "Evaluator.h"
#include "Evaluator.h"
#include "Evaluator.h"
#include "Evaluator.h"
#include "pch.h"

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
}

std::shared_ptr<IObject> Evaluator::Eval(const std::shared_ptr<Program>& program, const std::shared_ptr<Environment>& env, const std::shared_ptr<BuiltIn>& builtIn)  const
{
	std::shared_ptr<IObject> result = nullptr;
	for (const auto& stmt : program->Statements)
	{
		result = Eval(stmt, env, builtIn);
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
	else if (optor.Type == TokenType::TOKEN_EQ)
	{
		result = left->Value == right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_NOT_EQ)
	{
		result = left->Value != right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
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
	else if (optor.Type == TokenType::TOKEN_EQ)
	{
		result = abs(left->Value - right->Value) <= FLT_EPSILON ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (optor.Type == TokenType::TOKEN_NOT_EQ)
	{
		result = abs(left->Value - right->Value) > FLT_EPSILON ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
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

std::shared_ptr<IObject> Evaluator::MakeError(const std::string& message, const Token& token)
{
	return ErrorObj::New(message, token);
}

std::shared_ptr<Environment> Evaluator::ExtendFunctionEnv(const std::shared_ptr<FunctionObj>& func, const std::vector<std::shared_ptr<IObject>>& args)
{
	auto env = Environment::NewEnclosed(func->Env);
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
		env->Set(ident->Value, argUnwrapped);
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


std::shared_ptr<IObject> StackEvaluator::Eval(const std::shared_ptr<INode>& node, const std::shared_ptr<Environment>& env, const std::shared_ptr<BuiltIn>& builtIn) const
{
	std::stack<std::tuple<const INode*, int32_t, std::shared_ptr<Environment>>> stack;
	std::stack<std::shared_ptr<IObject>> results;

	stack.push({ node.get(), 0, nullptr});

	std::shared_ptr<Environment> useEnv = env;

	while (!stack.empty())
	{
		auto [currentNode, signal, subEnv] = stack.top();
		stack.pop();

		//check for errors
		if (!results.empty() && results.top()->Type() == ObjectType::ERROR_OBJ)
		{
			return results.top();
		}

		if (subEnv != nullptr)
		{
			useEnv = subEnv;
		}
		else
		{
			useEnv = env;
		}

		auto type = currentNode->NType();
		switch (type)
		{
		case NodeType::Program:
		{
			const auto* program = dynamic_cast<const Program*>(currentNode);

			for (const auto& iter : program->Statements | std::views::reverse)
			{
				stack.push({ iter.get(), 0, useEnv });
			}
			break;
		}
		case NodeType::ExpressionStatement:
		{
			const auto* expression = dynamic_cast<const ExpressionStatement*>(currentNode);
			stack.push({ expression->Expression.get(), 0, useEnv});
			break;
		}
		case NodeType::NullLiteral:
		{
			results.push(NullObj::NULL_OBJ_REF);
			break;
		}
		case NodeType::IntegerLiteral:
		{
			const auto* integer = dynamic_cast<const IntegerLiteral*>(currentNode);
			results.push(IntegerObj::New(integer->Value));
			break;
		}
		case NodeType::DecimalLiteral:
		{
			const auto* decimal = dynamic_cast<const DecimalLiteral*>(currentNode);
			results.push(DecimalObj::New(decimal->Value));
			break;
		}
		case NodeType::StringLiteral:
		{
			const auto* string = dynamic_cast<const StringLiteral*>(currentNode);
			results.push(StringObj::New(string->Value));
			break;
		}
		case NodeType::BooleanLiteral:
		{
			const auto* boolean = dynamic_cast<const BooleanLiteral*>(currentNode);
			results.push(boolean->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF);
			break;
		}
		case NodeType::ArrayLiteral:
		{
			const auto* array = dynamic_cast<const ArrayLiteral*>(currentNode);
			std::vector<std::shared_ptr<IObject>> elements;

			if (signal == 0)
			{
				stack.push({ currentNode, 1, useEnv });

				for (const auto& elem : array->Elements)
				{
					stack.push({ elem.get(), 0, useEnv });
				}
			}
			else
			{
				std::vector<std::shared_ptr<IObject>> evalArgs;
				if (results.size() < array->Elements.size())
				{
					results.push(MakeError("failed to evaluate all array elements", array->BaseToken));
					break;
				}

				for (size_t i = 0; i < array->Elements.size(); i++)
				{
					auto arg = results.top();
					results.pop();

					if (arg->Type() == ObjectType::ERROR_OBJ)
					{
						results.push(arg);
						break;
					}

					arg = UnwrapIfReturnObj(arg);
					arg = UnwrapIfIdentObj(arg);

					evalArgs.push_back(arg);
				}
				results.push(ArrayObj::New(evalArgs));
			}
			break;
		}
		case NodeType::HashLiteral:
		{
			const auto* hash = dynamic_cast<const HashLiteral*>(currentNode);
			std::vector<std::shared_ptr<IObject>> elements;

			if (signal == 0)
			{
				stack.push({ currentNode, 1, useEnv });

				for (const auto& [key, value] : hash->Elements)
				{
					stack.push({ key.get(), 0, useEnv});
					stack.push({ value.get(), 0, useEnv });
				}
			}
			else
			{
				std::unordered_map<HashKey, HashEntry> evalArgs;
				if (results.size() < hash->Elements.size())
				{
					results.push(MakeError("failed to evaluate all hash elements", hash->BaseToken));
					break;
				}

				for (size_t i = 0; i < hash->Elements.size(); i++)
				{
					auto key = results.top();
					results.pop();

					if (key->Type() == ObjectType::ERROR_OBJ)
					{
						results.push(key);
						break;
					}

					key = UnwrapIfReturnObj(key);
					key = UnwrapIfIdentObj(key);

					auto value = results.top();
					results.pop();

					if (value->Type() == ObjectType::ERROR_OBJ)
					{
						results.push(value);
						break;
					}

					value = UnwrapIfReturnObj(value);
					value = UnwrapIfIdentObj(value);

					evalArgs[HashKey{ key->Type(), key->Inspect() }] = HashEntry{ key, value };
				}
				results.push(HashObj::New(evalArgs));
			}
			break;
		}
		case NodeType::IndexExpression:
		{
			const auto* index = dynamic_cast<const IndexExpression*>(currentNode);
			if (signal == 0)
			{
				stack.push({ currentNode, 1, useEnv });
				stack.push({ index->Left.get(), 0, useEnv });
				stack.push({ index->Index.get(), 0, useEnv });
			}
			else
			{
				auto left = results.top();
				results.pop();

				auto indexObj = results.top();
				results.pop();

				left = UnwrapIfIdentObj(left);
				indexObj = UnwrapIfIdentObj(indexObj);

				auto result = EvalIndexExpression(index->BaseToken, left, indexObj);
				results.push(result);
			}
			break;
		}
		case NodeType::PrefixExpression:
		{
			const auto* prefix = dynamic_cast<const PrefixExpression*>(currentNode);
			if (signal == 0)
			{
				stack.push({ currentNode, 1, useEnv });
				stack.push({ prefix->Right.get(), 0, useEnv});
			}
			else
			{
				auto lastResult = results.top();
				results.pop();

				//unwrap any return objects
				lastResult = UnwrapIfReturnObj(lastResult);
				lastResult = UnwrapIfIdentObj(lastResult);

				results.push(EvalPrefixExpression(prefix->BaseToken, lastResult));
			}
			break;
		}
		case NodeType::BlockStatement:
		{
			const auto* block = dynamic_cast<const BlockStatement*>(currentNode);

			//check the action for return/error
			if (!results.empty() && (results.top()->Type() == ObjectType::ERROR_OBJ || results.top()->Type() == ObjectType::RETURN_OBJ))
			{
				break;
			}

			if (signal < block->Statements.size())
			{
				//get the statement for the current signal
				const auto& stmt = block->Statements[signal];

				stack.push({ currentNode, signal + 1, useEnv });
				stack.push({ stmt.get(), 0, useEnv });
			}
			break;
		}
		case NodeType::IfExpression:
		{
			const auto* ifex = dynamic_cast<const IfExpression*>(currentNode);
			if (signal == 0)
			{
				stack.push({ currentNode,  1, useEnv });
				stack.push({ ifex->Condition.get(), 0, useEnv });
			}
			else
			{
				auto lastResult = results.top();
				results.pop();

				//unwrap any return objects
				lastResult = UnwrapIfReturnObj(lastResult);
				lastResult = UnwrapIfIdentObj(lastResult);

				auto evalBool = EvalAsBoolean(ifex->BaseToken, lastResult.get());

				if (evalBool->Type() == ObjectType::ERROR_OBJ)
				{
					results.push(evalBool);
					break;
				}

				if (evalBool == BooleanObj::TRUE_OBJ_REF)
				{
					stack.push({ ifex->Consequence.get(), 0, useEnv });
				}
				else if (ifex->Alternative != nullptr)
				{
					stack.push({ ifex->Alternative.get(), 0, useEnv });
				}
				else
				{
					results.push(NullObj::NULL_OBJ_REF);
				}
			}
			break;
		}
		case NodeType::InfixExpression:
		{
			const auto* infix = dynamic_cast<const InfixExpression*>(currentNode);
			if (signal == 0)
			{
				stack.push({ currentNode, 1, useEnv });
				stack.push({ infix->Left.get(), 0, useEnv });
			}
			else if (signal == 1)
			{
				stack.push({ currentNode, 2, useEnv });
				stack.push({ infix->Right.get(), 0, useEnv });
			}
			else
			{
				if (results.size() < 2)
				{
					results.push(MakeError("infix expression requires two operands", infix->BaseToken));
					break;
				}

				auto right = results.top();
				results.pop();

				right = UnwrapIfReturnObj(right);
				right = UnwrapIfIdentObj(right);

				auto left = results.top();
				results.pop();

				left = UnwrapIfReturnObj(left);
				left = UnwrapIfIdentObj(left);

				auto result = EvalInfixExpression(infix->BaseToken, left, right);
				results.push(result);
			}
			break;
		}
		case NodeType::ReturnStatement:
		{
			const auto* ret = dynamic_cast<const ReturnStatement*>(currentNode);
			if (signal == 0)
			{
				stack.push({ currentNode, 1, useEnv });
				stack.push({ ret->ReturnValue.get(), 0, useEnv });
			}
			else
			{
				auto lastResult = results.top();
				results.pop();
				results.push(ReturnObj::New(lastResult));
			}
			break;
		}
		case NodeType::LetStatement:
		{
			const auto* let = dynamic_cast<const LetStatement*>(currentNode);
			if (let->Name->NType() == NodeType::Identifier)
			{
				if (signal == 0)
				{
					stack.push({ currentNode, 1, useEnv });
					stack.push({ let->Value.get(), 0, useEnv });
					stack.push({ let->Name.get(), 0, useEnv });
				}
				else
				{
					auto value = results.top();
					results.pop();

					value = UnwrapIfReturnObj(value);
					value = UnwrapIfIdentObj(value);

					if (value->Type() == ObjectType::ERROR_OBJ)
					{
						return value;
					}

					auto identResult = results.top();
					results.pop();

					identResult = UnwrapIfReturnObj(identResult);

					if (identResult->Type() == ObjectType::ERROR_OBJ)
					{
						return identResult;
					}


					auto* ident = dynamic_no_copy_cast<IdentifierObj>(identResult);
					auto result = ident->Set(nullptr, value);
					//results.push(result);
				}
			}
			else if (let->Name->NType() == NodeType::IndexExpression)
			{
				if (signal == 0)
				{
					auto* index = dynamic_no_copy_cast<IndexExpression>(let->Name);

					stack.push({ currentNode, 1, useEnv });
					stack.push({ let->Value.get(), 0, useEnv });
					stack.push({ index->Left.get(), 0, useEnv});
					stack.push({ index->Index.get(), 0, useEnv});
				}
				else
				{
					auto value = results.top();
					results.pop();

					value = UnwrapIfReturnObj(value);
					value = UnwrapIfIdentObj(value);

					if (value->Type() == ObjectType::ERROR_OBJ)
					{
						return value;
					}

					auto left = results.top();
					results.pop();

					left = UnwrapIfReturnObj(left);
					left = UnwrapIfIdentObj(left);

					if (left->Type() == ObjectType::ERROR_OBJ)
					{
						return left;
					}

					auto index = results.top();
					results.pop();

					index = UnwrapIfReturnObj(index);
					index = UnwrapIfIdentObj(index);

					if (index->Type() == ObjectType::ERROR_OBJ)
					{
						return index;
					}

					auto* assignable = dynamic_no_copy_cast<IAssignableObject>(left);
					auto result = assignable->Set(index, value);
					//results.push(result);
				}
			}
			else
			{
				results.push(MakeError("let target must be identifier or index expression", let->BaseToken));
			}
			break;
		}
		case NodeType::Identifier:
		{
			const auto* ident = dynamic_cast<const Identifier*>(currentNode);
			if (builtIn->IsBuiltIn(ident->Value))
			{
				results.push(BuiltInObj::New(ident->Value));
			}
			else
			{
				auto value = useEnv->Get(ident->Value);
				if (value != nullptr)
				{
					results.push(IdentifierObj::New(ident->Value, value, useEnv));
				}
				else
				{
					results.push(IdentifierObj::New(ident->Value, NullObj::NULL_OBJ_REF, useEnv));
				}
				//result = MakeError(std::format("identifier not found: {}", ident->Value), ident->BaseToken);
			}
			break;
		}
		case NodeType::FunctionLiteral:
		{
			const auto* func = dynamic_cast<const FunctionLiteral*>(currentNode);
			results.push(FunctionObj::New(func->Parameters, func->Body, useEnv));
			break;
		}
		case NodeType::CallExpression:
		{
			const auto* call = dynamic_cast<const CallExpression*>(currentNode);
			if (signal == 0)
			{
				stack.push({ currentNode, 1, useEnv });
				stack.push({ call->Function.get(), 0, useEnv });
			}
			else if (signal == 1)
			{
				auto& lastResult = results.top();

				lastResult = UnwrapIfIdentObj(lastResult);

				if (lastResult->Type() != ObjectType::FUNCTION_OBJ && lastResult->Type() != ObjectType::BUILTIN_OBJ)
				{
					results.push(MakeError(std::format("literal not a function: {}", lastResult->Inspect()), call->BaseToken));
					break;
				}
				auto function = lastResult;

				stack.push({ currentNode, 2, useEnv });

				// Evaluate arguments

				for (const auto& iter : call->Arguments)
				{
					stack.push({ iter.get(), 0, useEnv });
				}
			}
			else
			{
				std::vector<std::shared_ptr<IObject>> evalArgs;

				if (results.size() < call->Arguments.size())
				{
					results.push(MakeError("not enough arguments", call->BaseToken));
					break;
				}

				for (size_t i = 0; i < call->Arguments.size(); i++)
				{
					auto arg = results.top();
					results.pop();

					if (arg->Type() == ObjectType::ERROR_OBJ)
					{
						results.push(arg);
						break;
					}

					//unwrap any return objects
					arg = UnwrapIfReturnObj(arg);
					arg = UnwrapIfIdentObj(arg);

					evalArgs.push_back(arg);
				}
				auto function = results.top();
				results.pop();

				if (function->Type() == ObjectType::BUILTIN_OBJ)
				{
					auto func = std::dynamic_pointer_cast<BuiltInObj>(function);
					auto builtInToCall = builtIn->GetBuiltInFunction(func->Name);
					if (builtInToCall == nullptr)
					{
						results.push(MakeError(std::format("unknown function: {}", func->Name), call->BaseToken));
						break;
					}
					auto result = builtInToCall(evalArgs, call->BaseToken);

					if (result != VoidObj::VOID_OBJ_REF) //check for a "void" return
					{
						results.push(result);
					}
				}
				else
				{
					auto func = std::dynamic_pointer_cast<FunctionObj>(function);
					auto extEnv = ExtendFunctionEnv(func, evalArgs);
					stack.push({ func->Body.get(), 0, extEnv });
				}
			}
			break;
		}
		case NodeType::WhileExpression:
		{
			const auto* whileEx = dynamic_cast<const WhileExpression*>(currentNode);
			if (signal == 0)
			{
				//check the action for return/error
				if (!results.empty() && (results.top()->Type() == ObjectType::ERROR_OBJ || results.top()->Type() == ObjectType::RETURN_OBJ))
				{
					if (results.top()->Type() == ObjectType::RETURN_OBJ)
					{
						auto* ret = dynamic_no_copy_cast<ReturnObj>(results.top());
						if (ret->Value == ContinueObj::CONTINUE_OBJ_REF)
						{
							results.pop();
						}
						else
						{
							if (ret->Value == BreakObj::BREAK_OBJ_REF)
							{
								results.pop();
							}
							break;
						}
					}
					
					if (results.top()->Type() == ObjectType::ERROR_OBJ)
					{
						break;
					}
				}
				stack.push({ currentNode, 1, useEnv });
				stack.push({ whileEx->Condition.get(), 0, useEnv });
			}
			else
			{
				auto lastResult = results.top();
				results.pop();

				lastResult = UnwrapIfReturnObj(lastResult);
				lastResult = UnwrapIfIdentObj(lastResult);

				auto booleanObj = EvalAsBoolean(whileEx->BaseToken, lastResult.get());

				if (booleanObj->Type() == ObjectType::ERROR_OBJ)
				{
					results.push(booleanObj);
					break;
				}

				if (booleanObj == BooleanObj::TRUE_OBJ_REF)
				{
					stack.push({ currentNode, 0, useEnv });
					stack.push({ whileEx->Action.get(), 0, useEnv });
				}
			}
			break;
		}
		case NodeType::ForExpression:
		{
			const auto* forEx = dynamic_cast<const ForExpression*>(currentNode);
			if (signal == 0)
			{
				stack.push({ currentNode, 1, useEnv });
				stack.push({ forEx->Init.get(), 0, useEnv });
			}
			else if (signal == 1)
			{
				//condition
				stack.push({ currentNode, 2, useEnv });
				stack.push({ forEx->Condition.get(), 0, useEnv });
			}
			else if (signal == 2)
			{
				auto lastResult = results.top();
				results.pop();

				lastResult = UnwrapIfReturnObj(lastResult);
				lastResult = UnwrapIfIdentObj(lastResult);

				auto booleanObj = EvalAsBoolean(forEx->BaseToken, lastResult.get());

				if (booleanObj->Type() == ObjectType::ERROR_OBJ)
				{
					results.push(booleanObj);
					break;
				}

				if (booleanObj == BooleanObj::TRUE_OBJ_REF)
				{
					stack.push({ currentNode, 3, useEnv });
					stack.push({ forEx->Action.get(), 0, useEnv });
				}
			}
			else if (signal == 3)
			{
				//check the action for return/error
				if (!results.empty() && (results.top()->Type() == ObjectType::ERROR_OBJ || results.top()->Type() == ObjectType::RETURN_OBJ))
				{
					if (results.top()->Type() == ObjectType::RETURN_OBJ)
					{
						auto* ret = dynamic_no_copy_cast<ReturnObj>(results.top());
						if (ret->Value == ContinueObj::CONTINUE_OBJ_REF)
						{
							results.pop();
						}
						else
						{
							if (ret->Value == BreakObj::BREAK_OBJ_REF)
							{
								results.pop();
							}
							break;
						}
					}

					if (results.top()->Type() == ObjectType::ERROR_OBJ)
					{
						break;
					}
				}
				stack.push({ currentNode, 1, useEnv });
				stack.push({ forEx->Post.get(), 0, useEnv });
			}
			break;
		}
		case NodeType::BreakStatement:
		{
			results.push(ReturnObj::New(BreakObj::BREAK_OBJ_REF));
			break;
		}
		case NodeType::ContinueStatement:
		{
			results.push(ReturnObj::New(ContinueObj::CONTINUE_OBJ_REF));
			break;
		}
		default:
		{
			results.push(NullObj::NULL_OBJ_REF);
		}
		}
	}

	if (results.empty())
	{
		return nullptr;
	}

	return results.top();
}


std::shared_ptr<IObject> RecursiveEvaluator::Eval(const std::shared_ptr<INode>& node, const std::shared_ptr<Environment>& env, const std::shared_ptr<BuiltIn>& builtIn) const
{
	auto type = node->NType();
	std::shared_ptr<IObject> result = nullptr;
	switch (type)
	{
		case NodeType::Program:
		{
			auto program = std::dynamic_pointer_cast<Program>(node);
			result = Eval(program, env, builtIn);
			break;
		}
		case NodeType::ExpressionStatement:
		{
			auto* expression = dynamic_no_copy_cast<ExpressionStatement>(node);
			result = Eval(expression->Expression, env, builtIn);
			break;
		}
		case NodeType::NullLiteral:
		{
			result = NullObj::NULL_OBJ_REF;
			break;
		}
		case NodeType::IntegerLiteral:
		{
			auto* integer = dynamic_no_copy_cast<IntegerLiteral>(node);
			result = IntegerObj::New(integer->Value);
			break;
		}
		case NodeType::StringLiteral:
		{
			auto* str = dynamic_no_copy_cast<StringLiteral>(node);
			result = StringObj::New(str->Value);
			break;
		}
		case NodeType::DecimalLiteral:
		{
			auto* decimal = dynamic_no_copy_cast<DecimalLiteral>(node);
			result = DecimalObj::New(decimal->Value);
			break;
		}
		case NodeType::BooleanLiteral:
		{
			auto* boolean = dynamic_no_copy_cast<BooleanLiteral>(node);
			result = boolean->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
			break;
		}
		case NodeType::ArrayLiteral:
		{
			auto* array = dynamic_no_copy_cast<ArrayLiteral>(node);
			std::vector<std::shared_ptr<IObject>> elements;
			for (const auto& elem : array->Elements)
			{
				auto eval = Eval(elem, env, builtIn);
				if (eval->Type() == ObjectType::ERROR_OBJ)
				{
					return eval;
				}
				elements.push_back(eval);
			}
			result = ArrayObj::New(elements);
			break;
		}
		case NodeType::HashLiteral:
		{
			auto* hash = dynamic_no_copy_cast<HashLiteral>(node);
			std::unordered_map<HashKey, HashEntry> elems;
			for (const auto& [key, value] : hash->Elements)
			{
				auto keyObj = Eval(key, env, builtIn);
				if (keyObj->Type() == ObjectType::ERROR_OBJ)
				{
					return keyObj;
				}

				auto valueObj = Eval(value, env, builtIn);
				if (valueObj->Type() == ObjectType::ERROR_OBJ)
				{
					return valueObj;
				}
				elems[HashKey{ keyObj->Type(), keyObj->Inspect() }] = HashEntry{ keyObj, valueObj };
			}
			result = HashObj::New(elems);
			break;
		}
		case NodeType::PrefixExpression:
		{
			auto* prefix = dynamic_no_copy_cast<PrefixExpression>(node);
			auto right = Eval(prefix->Right, env, builtIn);

			right = UnwrapIfReturnObj(right);
			right = UnwrapIfIdentObj(right);

			if (right->Type() == ObjectType::ERROR_OBJ)
			{
				return right;
			}
			result = EvalPrefixExpression(prefix->BaseToken, right);
			break;
		}
		case NodeType::BlockStatement:
		{
			auto* block = dynamic_no_copy_cast<BlockStatement>(node);
			for (const auto& stmt : block->Statements)
			{
				result = Eval(stmt, env, builtIn);
				if (result == nullptr)
				{
					continue;
				}

				if (result->Type() == ObjectType::RETURN_OBJ || result->Type() == ObjectType::ERROR_OBJ)
				{
					break;
				}
			}
			break;
		}
		case NodeType::IfExpression:
		{
			auto* ifex = dynamic_no_copy_cast<IfExpression>(node);

			auto condition = Eval(ifex->Condition, env, builtIn);

			condition = UnwrapIfReturnObj(condition);
			condition = UnwrapIfIdentObj(condition);

			if (condition->Type() == ObjectType::ERROR_OBJ)
			{
				return condition;
			}

			auto booleanObj = EvalAsBoolean(ifex->BaseToken, condition.get());

			if (booleanObj->Type() == ObjectType::ERROR_OBJ)
			{
				return booleanObj;
			}

			if (booleanObj == BooleanObj::TRUE_OBJ_REF)
			{
				result = Eval(ifex->Consequence, env, builtIn);
			}
			else if (ifex->Alternative != nullptr)
			{
				result = Eval(ifex->Alternative, env, builtIn);
			}
			else
			{
				result = NullObj::NULL_OBJ_REF;
			}
			break;
		}
		case NodeType::InfixExpression:
		{
			auto* infix = dynamic_no_copy_cast<InfixExpression>(node);
			auto left = Eval(infix->Left, env, builtIn);

			left = UnwrapIfReturnObj(left);
			left = UnwrapIfIdentObj(left);

			if (left->Type() == ObjectType::ERROR_OBJ)
			{
				return left;
			}
			auto right = Eval(infix->Right, env, builtIn);

			right = UnwrapIfReturnObj(right);
			right = UnwrapIfIdentObj(right);

			if (right->Type() == ObjectType::ERROR_OBJ)
			{
				return right;
			}
			result = EvalInfixExpression(infix->BaseToken, left, right);
			break;
		}
		case NodeType::ReturnStatement:
		{
			auto* ret = dynamic_no_copy_cast<ReturnStatement>(node);
			auto value = Eval(ret->ReturnValue, env, builtIn);
			result = ReturnObj::New(value);
			break;
		}
		case NodeType::LetStatement:
		{
			auto* let = dynamic_no_copy_cast<LetStatement>(node);
			auto value = Eval(let->Value, env, builtIn);

			value = UnwrapIfReturnObj(value);
			value = UnwrapIfIdentObj(value);

			if (value->Type() == ObjectType::ERROR_OBJ)
			{
				return value;
			}

			if (let->Name->NType() == NodeType::Identifier)
			{
				auto target = Eval(let->Name, env, builtIn);
				if (target->Type() == ObjectType::ERROR_OBJ)
				{
					return target;
				}
				auto* ident = dynamic_no_copy_cast<IdentifierObj>(target);
				ident->Set(nullptr, value);
			}
			else if (let->Name->NType() == NodeType::IndexExpression)
			{
				auto* index = dynamic_no_copy_cast<IndexExpression>(let->Name);
				auto left = Eval(index->Left, env, builtIn);
				auto indexObj = Eval(index->Index, env, builtIn);

				left = UnwrapIfReturnObj(left);
				left = UnwrapIfIdentObj(left);

				if (left->Type() == ObjectType::ERROR_OBJ)
				{
					return left;
				}

				indexObj = UnwrapIfReturnObj(indexObj);
				indexObj = UnwrapIfIdentObj(indexObj);

				if (indexObj->Type() == ObjectType::ERROR_OBJ)
				{
					return indexObj;
				}

				auto* assignable = dynamic_no_copy_cast<IAssignableObject>(left);
				if (assignable == nullptr)
				{
					return MakeError("left side of assignment is not assignable", index->BaseToken);
				}
				assignable->Set(indexObj, value);
			}
			else
			{
				return MakeError("let target must be identifier or index expression", let->BaseToken);
			}
			break;
		}
		case NodeType::Identifier:
		{
			auto* ident = dynamic_no_copy_cast<Identifier>(node);

			if (builtIn->IsBuiltIn(ident->Value))
			{
				result = BuiltInObj::New(ident->Value);
			}
			else
			{
				auto value = env->Get(ident->Value);
				if (value != nullptr)
				{
					result = IdentifierObj::New(ident->Value, value, env);
				}
				else
				{
					result = IdentifierObj::New(ident->Value, NullObj::NULL_OBJ_REF, env);
				}
				//result = MakeError(std::format("identifier not found: {}", ident->Value), ident->BaseToken);
			}
			break;
		}
		case NodeType::IndexExpression:
		{
			auto* index = dynamic_no_copy_cast<IndexExpression>(node);
			auto left = Eval(index->Left, env, builtIn);
			auto indexObj = Eval(index->Index, env, builtIn);

			left = UnwrapIfReturnObj(left);
			left = UnwrapIfIdentObj(left);

			if (left->Type() == ObjectType::ERROR_OBJ)
			{
				return left;
			}

			indexObj = UnwrapIfReturnObj(indexObj);
			indexObj = UnwrapIfIdentObj(indexObj);

			if (indexObj->Type() == ObjectType::ERROR_OBJ)
			{
				return indexObj;
			}

			result = EvalIndexExpression(index->BaseToken, left, indexObj);
			break;
		}
		case NodeType::FunctionLiteral:
		{
			auto* func = dynamic_no_copy_cast<FunctionLiteral>(node);
			result = FunctionObj::New(func->Parameters, func->Body, env);
			break;
		}
		case NodeType::CallExpression:
		{
			const auto* call = dynamic_no_copy_cast<CallExpression>(node);
			auto function = Eval(call->Function, env, builtIn);
			if (function->Type() == ObjectType::ERROR_OBJ)
			{
				return function;
			}

			function = UnwrapIfIdentObj(function);

			if (function->Type() != ObjectType::FUNCTION_OBJ && function->Type() != ObjectType::BUILTIN_OBJ)
			{
				return MakeError(std::format("literal not a function or builtin: {}", function->Inspect()), call->BaseToken);
			}

			auto evalArgs = EvalExpressions(call->Arguments, env, builtIn);
			if (evalArgs.size() == 1 && evalArgs[0]->Type() == ObjectType::ERROR_OBJ)
			{
				return evalArgs[0];
			}

			if (function->Type() == ObjectType::BUILTIN_OBJ)
			{
				auto* builtInObj = dynamic_no_copy_cast<BuiltInObj>(function);
				auto builtInToCall = builtIn->GetBuiltInFunction(builtInObj->Name);

				if (builtInToCall == nullptr)
				{
					return MakeError(std::format("builtin function not found: {}", builtInObj->Name), call->BaseToken);
				}

				result = builtInToCall(evalArgs, call->BaseToken);
			}
			else
			{
				auto func = std::dynamic_pointer_cast<FunctionObj>(function);
				result = ApplyFunction(func, evalArgs, builtIn);
			}
			break;
		}
		case NodeType::WhileExpression:
		{
			auto* whileEx = dynamic_no_copy_cast<WhileExpression>(node);
			auto condition = Eval(whileEx->Condition, env, builtIn);

			condition = UnwrapIfReturnObj(condition);
			condition = UnwrapIfIdentObj(condition);

			if (condition->Type() == ObjectType::ERROR_OBJ)
			{
				return condition;
			}

			auto booleanObj = EvalAsBoolean(whileEx->BaseToken, condition.get());

			if (booleanObj->Type() == ObjectType::ERROR_OBJ)
			{
				return booleanObj;
			}

			while (booleanObj == BooleanObj::TRUE_OBJ_REF)
			{
				auto evaluated = Eval(whileEx->Action, env, builtIn);
				if (evaluated != nullptr)
				{
					if (evaluated->Type() == ObjectType::RETURN_OBJ)
					{
						auto* ret = dynamic_no_copy_cast<ReturnObj>(evaluated);
						if (ret->Value == BreakObj::BREAK_OBJ_REF)
						{
							break;
						}
						
						if (ret->Value != ContinueObj::CONTINUE_OBJ_REF)
						{
							return evaluated;
						}
					}

					if (evaluated->Type() == ObjectType::ERROR_OBJ)
					{
						return evaluated;
					}
				}
				condition = Eval(whileEx->Condition, env, builtIn);

				condition = UnwrapIfReturnObj(condition);
				condition = UnwrapIfIdentObj(condition);

				if (condition->Type() == ObjectType::ERROR_OBJ)
				{
					return condition;
				}

				booleanObj = EvalAsBoolean(whileEx->BaseToken, condition.get());

				if (booleanObj->Type() == ObjectType::ERROR_OBJ)
				{
					return booleanObj;
				}
			}
			result = NullObj::NULL_OBJ_REF;
			break;
		}
		case NodeType::ForExpression:
		{
			auto* forEx = dynamic_no_copy_cast<ForExpression>(node);

			auto init = Eval(forEx->Init, env, builtIn);

			auto condition = Eval(forEx->Condition, env, builtIn);

			condition = UnwrapIfReturnObj(condition);
			condition = UnwrapIfIdentObj(condition);

			if (condition->Type() == ObjectType::ERROR_OBJ)
			{
				return condition;
			}

			auto booleanObj = EvalAsBoolean(forEx->BaseToken, condition.get());

			if (booleanObj->Type() == ObjectType::ERROR_OBJ)
			{
				return booleanObj;
			}

			while (booleanObj == BooleanObj::TRUE_OBJ_REF)
			{
				auto evaluated = Eval(forEx->Action, env, builtIn);
				if (evaluated != nullptr)
				{
					if (evaluated->Type() == ObjectType::RETURN_OBJ)
					{
						auto* ret = dynamic_no_copy_cast<ReturnObj>(evaluated);
						if (ret->Value == BreakObj::BREAK_OBJ_REF)
						{
							break;
						}

						if (ret->Value != ContinueObj::CONTINUE_OBJ_REF)
						{
							return evaluated;
						}
					}

					if (evaluated->Type() == ObjectType::ERROR_OBJ)
					{
						return evaluated;
					}
				}

				auto post = Eval(forEx->Post, env, builtIn);

				condition = Eval(forEx->Condition, env, builtIn);

				condition = UnwrapIfReturnObj(condition);
				condition = UnwrapIfIdentObj(condition);

				if (condition->Type() == ObjectType::ERROR_OBJ)
				{
					return condition;
				}

				booleanObj = EvalAsBoolean(forEx->BaseToken, condition.get());

				if (booleanObj->Type() == ObjectType::ERROR_OBJ)
				{
					return booleanObj;
				}
			}
			result = NullObj::NULL_OBJ_REF;
			break;
		}
		case NodeType::BreakStatement:
		{
			result = ReturnObj::New(BreakObj::BREAK_OBJ_REF);
			break;
		}
		case NodeType::ContinueStatement:
		{
			result = ReturnObj::New(ContinueObj::CONTINUE_OBJ_REF);
			break;
		}
		default:
		{
			result = NullObj::NULL_OBJ_REF;
		}
	}

	if (VoidObj::VOID_OBJ_REF == result)
	{
		return nullptr;
	}

	return result;
}

std::vector<std::shared_ptr<IObject>> RecursiveEvaluator::EvalExpressions(const std::vector<std::shared_ptr<IExpression>>& expressions, const std::shared_ptr<Environment>& env, const std::shared_ptr<BuiltIn>& builtIn) const
{
	std::vector<std::shared_ptr<IObject>> result;
	for (const auto& expr : expressions)
	{
		auto evaluated = Eval(expr, env, builtIn);
		if (evaluated->Type() == ObjectType::ERROR_OBJ)
		{
			return { evaluated };
		}

		if (evaluated->Type() == ObjectType::RETURN_OBJ)
		{
			auto ret = std::dynamic_pointer_cast<ReturnObj>(evaluated);

			auto unwrapped = UnwrapIfReturnObj(ret->Value);
			unwrapped = UnwrapIfIdentObj(unwrapped);
			
			result.push_back(unwrapped);
		}
		else if (evaluated->Type() == ObjectType::IDENT_OBJ)
		{
			auto ident = std::dynamic_pointer_cast<IdentifierObj>(evaluated);
			auto unwrapped = UnwrapIfIdentObj(ident);
			result.push_back(unwrapped);
		}
		else
		{
			result.push_back(evaluated);
		}
	}
	return result;
}

std::shared_ptr<IObject> RecursiveEvaluator::ApplyFunction(const std::shared_ptr<FunctionObj>& func, const std::vector<std::shared_ptr<IObject>>& args, const std::shared_ptr<BuiltIn>& builtIn) const
{
	auto extEnv = ExtendFunctionEnv(func, args);
	auto evaluated = Eval(func->Body, extEnv, builtIn);
	return evaluated;
}