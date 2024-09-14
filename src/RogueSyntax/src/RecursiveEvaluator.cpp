#pragma once
#include "pch.h"
#include "RecursiveEvaluator.h"


std::shared_ptr<IObject> RecursiveEvaluator::Eval(const std::shared_ptr<INode>& node, const uint32_t env)
{
	auto type = node->NType();
	std::shared_ptr<IObject> result = nullptr;
	switch (type)
	{
	case NodeType::Program:
	{
		auto program = std::dynamic_pointer_cast<Program>(node);
		result = Eval(program, env);
		break;
	}
	case NodeType::ExpressionStatement:
	{
		auto* expression = dynamic_no_copy_cast<ExpressionStatement>(node);
		result = Eval(expression->Expression, env);
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
			auto eval = Eval(elem, env);
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
			auto keyObj = Eval(key, env);
			if (keyObj->Type() == ObjectType::ERROR_OBJ)
			{
				return keyObj;
			}

			auto valueObj = Eval(value, env);
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
		auto right = Eval(prefix->Right, env);

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
			result = Eval(stmt, env);
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

		auto condition = Eval(ifex->Condition, env);

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
			result = Eval(ifex->Consequence, env);
		}
		else if (ifex->Alternative != nullptr)
		{
			result = Eval(ifex->Alternative, env);
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
		auto left = Eval(infix->Left, env);

		left = UnwrapIfReturnObj(left);
		left = UnwrapIfIdentObj(left);

		if (left->Type() == ObjectType::ERROR_OBJ)
		{
			return left;
		}
		auto right = Eval(infix->Right, env);

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
		auto value = Eval(ret->ReturnValue, env);
		result = ReturnObj::New(value);
		break;
	}
	case NodeType::LetStatement:
	{
		auto* let = dynamic_no_copy_cast<LetStatement>(node);
		auto value = Eval(let->Value, env);

		value = UnwrapIfReturnObj(value);
		value = UnwrapIfIdentObj(value);

		if (value->Type() == ObjectType::ERROR_OBJ)
		{
			return value;
		}

		if (let->Name->NType() == NodeType::Identifier)
		{
			auto target = Eval(let->Name, env);
			if (target->Type() == ObjectType::ERROR_OBJ)
			{
				return target;
			}
			auto* ident = dynamic_no_copy_cast<IdentifierObj>(target);
			ident->Set(nullptr, value);
			EvalEnvironment->Set(env, ident->Name, value);
		}
		else if (let->Name->NType() == NodeType::IndexExpression)
		{
			auto* index = dynamic_no_copy_cast<IndexExpression>(let->Name);
			auto left = Eval(index->Left, env);
			auto indexObj = Eval(index->Index, env);

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

		if (EvalBuiltIn->IsBuiltIn(ident->Value))
		{
			result = BuiltInObj::New(ident->Value);
		}
		else
		{
			auto value = EvalEnvironment->Get(env, ident->Value);
			if (value != nullptr)
			{
				result = IdentifierObj::New(ident->Value, value);
			}
			else
			{
				result = IdentifierObj::New(ident->Value, NullObj::NULL_OBJ_REF);
			}
			//result = MakeError(std::format("identifier not found: {}", ident->Value), ident->BaseToken);
		}
		break;
	}
	case NodeType::IndexExpression:
	{
		auto* index = dynamic_no_copy_cast<IndexExpression>(node);
		auto left = Eval(index->Left, env);
		auto indexObj = Eval(index->Index, env);

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
		result = FunctionObj::New(func->Parameters, func->Body);
		break;
	}
	case NodeType::CallExpression:
	{
		const auto* call = dynamic_no_copy_cast<CallExpression>(node);
		auto function = Eval(call->Function, env);
		if (function->Type() == ObjectType::ERROR_OBJ)
		{
			return function;
		}

		function = UnwrapIfIdentObj(function);

		if (function->Type() != ObjectType::FUNCTION_OBJ && function->Type() != ObjectType::BUILTIN_OBJ)
		{
			return MakeError(std::format("literal not a function or builtin: {}", function->Inspect()), call->BaseToken);
		}

		auto evalArgs = EvalExpressions(call->Arguments, env);
		if (evalArgs.size() == 1 && evalArgs[0]->Type() == ObjectType::ERROR_OBJ)
		{
			return evalArgs[0];
		}

		if (function->Type() == ObjectType::BUILTIN_OBJ)
		{
			auto* builtInObj = dynamic_no_copy_cast<BuiltInObj>(function);
			auto builtInToCall = EvalBuiltIn->GetBuiltInFunction(builtInObj->Name);

			if (builtInToCall == nullptr)
			{
				return MakeError(std::format("builtin function not found: {}", builtInObj->Name), call->BaseToken);
			}

			result = builtInToCall(evalArgs, call->BaseToken);
		}
		else
		{
			auto func = std::dynamic_pointer_cast<FunctionObj>(function);
			result = ApplyFunction(env, func, evalArgs);
		}
		break;
	}
	case NodeType::WhileExpression:
	{
		auto* whileEx = dynamic_no_copy_cast<WhileExpression>(node);
		auto condition = Eval(whileEx->Condition, env);

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
			auto evaluated = Eval(whileEx->Action, env);
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
			condition = Eval(whileEx->Condition, env);

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

		auto init = Eval(forEx->Init, env);

		auto condition = Eval(forEx->Condition, env);

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
			auto evaluated = Eval(forEx->Action, env);
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

			auto post = Eval(forEx->Post, env);

			condition = Eval(forEx->Condition, env);

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


std::vector<std::shared_ptr<IObject>> RecursiveEvaluator::EvalExpressions(const std::vector<std::shared_ptr<IExpression>>& expressions, const uint32_t env)
{
	std::vector<std::shared_ptr<IObject>> result;
	for (const auto& expr : expressions)
	{
		auto evaluated = Eval(expr, env);
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
std::shared_ptr<IObject> RecursiveEvaluator::ApplyFunction(const uint32_t env, const std::shared_ptr<FunctionObj>& func, const std::vector<std::shared_ptr<IObject>>& args)
{
	auto extEnv = ExtendFunctionEnv(env, func, args);
	auto evaluated = Eval(func->Body, extEnv);
	return evaluated;
}