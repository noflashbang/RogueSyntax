#pragma once
#include "pch.h"
#include "RecursiveEvaluator.h"


std::shared_ptr<IObject> RecursiveEvaluator::Eval(const std::shared_ptr<INode>& node, const uint32_t env)
{
	_env = env; // set the environment
	auto result = Eval(node.get());

	if (VoidObj::VOID_OBJ_REF == result)
	{
		return nullptr;
	}

	return result;
}

std::shared_ptr<IObject> RecursiveEvaluator::Eval(INode* node)
{
	node->Eval(this);
	
	if (_results.empty())
	{
		return nullptr;
	}

	auto top = _results.top();
	_results.pop();
	return top;
}

void RecursiveEvaluator::NodeEval(Program* program)
{
	_results.push(Eval(program));
}

void RecursiveEvaluator::NodeEval(BlockStatement* block)
{
	for (const auto& stmt : block->Statements)
	{
		auto result = Eval(stmt, _env);
		_results.push(result);
		if (result == nullptr)
		{
			continue;
		}

		if (result->Type() == ObjectType::RETURN_OBJ || result->Type() == ObjectType::ERROR_OBJ)
		{
			break;
		}
	}
}

void RecursiveEvaluator::NodeEval(ExpressionStatement* expression)
{
	_results.push(Eval(expression->Expression, _env));
}

void RecursiveEvaluator::NodeEval(ReturnStatement* ret)
{
	auto value = Eval(ret->ReturnValue, _env);
	_results.push(ReturnObj::New(value));
}

void RecursiveEvaluator::NodeEval(LetStatement* let)
{
	auto value = Eval(let->Value, _env);

	value = UnwrapIfReturnObj(value);
	value = UnwrapIfIdentObj(value);

	if (value->Type() == ObjectType::ERROR_OBJ)
	{
		_results.push(value);
		return;
	}

	if (typeid(*(let->Name.get())) == typeid(Identifier))
	{
		auto target = Eval(let->Name, _env);
		if (target->Type() == ObjectType::ERROR_OBJ)
		{
			_results.push(target);
			return;
		}
		auto* ident = dynamic_no_copy_cast<IdentifierObj>(target);
		ident->Set(nullptr, value);
		EvalEnvironment->Set(_env, ident->Name, value);
	}
	else if (typeid(*(let->Name.get())) == typeid(IndexExpression))
	{
		auto* index = dynamic_no_copy_cast<IndexExpression>(let->Name);
		auto left = Eval(index->Left, _env);
		auto indexObj = Eval(index->Index, _env);

		left = UnwrapIfReturnObj(left);
		left = UnwrapIfIdentObj(left);

		if (left->Type() == ObjectType::ERROR_OBJ)
		{
			_results.push(left);
			return;
		}

		indexObj = UnwrapIfReturnObj(indexObj);
		indexObj = UnwrapIfIdentObj(indexObj);

		if (indexObj->Type() == ObjectType::ERROR_OBJ)
		{
			_results.push(indexObj);
			return;
		}

		auto* assignable = dynamic_no_copy_cast<IAssignableObject>(left);
		if (assignable == nullptr)
		{
			_results.push(MakeError("left side of assignment is not assignable", index->BaseToken));
			return;
		}
		assignable->Set(indexObj, value);
	}
	else
	{
		_results.push(MakeError("let target must be identifier or index expression", let->BaseToken));
	}
}

void RecursiveEvaluator::NodeEval(Identifier* ident)
{
	if (EvalBuiltIn->IsBuiltIn(ident->Value))
	{
		_results.push(BuiltInObj::New(ident->Value));
	}
	else
	{
		auto value = EvalEnvironment->Get(_env, ident->Value);
		if (value != nullptr)
		{
			_results.push(IdentifierObj::New(ident->Value, value));
		}
		else
		{
			_results.push(IdentifierObj::New(ident->Value, NullObj::NULL_OBJ_REF));
		}
	}
}

void RecursiveEvaluator::NodeEval(IntegerLiteral* integer)
{
	_results.push(IntegerObj::New(integer->Value));
}

void RecursiveEvaluator::NodeEval(BooleanLiteral* boolean)
{
	_results.push(boolean->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF);
}

void RecursiveEvaluator::NodeEval(StringLiteral* string)
{
	_results.push(StringObj::New(string->Value));
}

void RecursiveEvaluator::NodeEval(DecimalLiteral* decimal)
{
	_results.push(DecimalObj::New(decimal->Value));
}

void RecursiveEvaluator::NodeEval(PrefixExpression* prefix)
{
	auto right = Eval(prefix->Right, _env);

	right = UnwrapIfReturnObj(right);
	right = UnwrapIfIdentObj(right);

	if (right->Type() == ObjectType::ERROR_OBJ)
	{
		_results.push(right);
		return;
	}
	_results.push(EvalPrefixExpression(prefix->BaseToken, right));
}

void RecursiveEvaluator::NodeEval(InfixExpression* infix)
{
	auto left = Eval(infix->Left, _env);

	left = UnwrapIfReturnObj(left);
	left = UnwrapIfIdentObj(left);

	if (left->Type() == ObjectType::ERROR_OBJ)
	{
		_results.push(left);
		return;
	}
	auto right = Eval(infix->Right, _env);

	right = UnwrapIfReturnObj(right);
	right = UnwrapIfIdentObj(right);

	if (right->Type() == ObjectType::ERROR_OBJ)
	{
		_results.push(right);
		return;
	}
	_results.push(EvalInfixExpression(infix->BaseToken, left, right));
}

void RecursiveEvaluator::NodeEval(IfExpression* ifExpr)
{
	auto condition = Eval(ifExpr->Condition, _env);

	condition = UnwrapIfReturnObj(condition);
	condition = UnwrapIfIdentObj(condition);

	if (condition->Type() == ObjectType::ERROR_OBJ)
	{
		_results.push(condition);
		return;
	}

	auto booleanObj = EvalAsBoolean(ifExpr->BaseToken, condition.get());

	if (booleanObj->Type() == ObjectType::ERROR_OBJ)
	{
		_results.push(booleanObj);
		return;
	}

	if (booleanObj == BooleanObj::TRUE_OBJ_REF)
	{
		_results.push(Eval(ifExpr->Consequence, _env));
	}
	else if (ifExpr->Alternative != nullptr)
	{
		_results.push(Eval(ifExpr->Alternative, _env));
	}
	else
	{
		_results.push(NullObj::NULL_OBJ_REF);
	}
}

void RecursiveEvaluator::NodeEval(FunctionLiteral* function)
{
	_results.push(FunctionObj::New(function->Parameters, function->Body));
}

void RecursiveEvaluator::NodeEval(CallExpression* call)
{
	auto function = Eval(call->Function, _env);
	if (function->Type() == ObjectType::ERROR_OBJ)
	{
		_results.push(function);
		return;
	}

	function = UnwrapIfIdentObj(function);

	if (function->Type() != ObjectType::FUNCTION_OBJ && function->Type() != ObjectType::BUILTIN_OBJ)
	{
		_results.push(MakeError(std::format("literal not a function or builtin: {}", function->Inspect()), call->BaseToken));
		return;
	}

	auto evalArgs = EvalExpressions(call->Arguments);
	if (evalArgs.size() == 1 && evalArgs[0]->Type() == ObjectType::ERROR_OBJ)
	{
		_results.push(evalArgs[0]);
		return;
	}

	if (function->Type() == ObjectType::BUILTIN_OBJ)
	{
		auto* builtInObj = dynamic_no_copy_cast<BuiltInObj>(function);
		auto builtInToCall = EvalBuiltIn->GetBuiltInFunction(builtInObj->Name);

		if (builtInToCall == nullptr)
		{
			_results.push(MakeError(std::format("builtin function not found: {}", builtInObj->Name), call->BaseToken));
			return;
		}

		_results.push(builtInToCall(evalArgs, call->BaseToken));
	}
	else
	{
		auto func = std::dynamic_pointer_cast<FunctionObj>(function);
		_results.push(ApplyFunction(func, evalArgs));
	}
}

void RecursiveEvaluator::NodeEval(ArrayLiteral* array)
{
	std::vector<std::shared_ptr<IObject>> elements;
	for (const auto& elem : array->Elements)
	{
		auto eval = Eval(elem, _env);
		if (eval->Type() == ObjectType::ERROR_OBJ)
		{
			_results.push(eval);
			return;
		}
		elements.push_back(eval);
	}
	_results.push(ArrayObj::New(elements));
}

void RecursiveEvaluator::NodeEval(IndexExpression* index)
{
	auto left = Eval(index->Left, _env);
	auto indexObj = Eval(index->Index, _env);

	left = UnwrapIfReturnObj(left);
	left = UnwrapIfIdentObj(left);

	if (left->Type() == ObjectType::ERROR_OBJ)
	{
		_results.push(left);
		return;
	}

	indexObj = UnwrapIfReturnObj(indexObj);
	indexObj = UnwrapIfIdentObj(indexObj);

	if (indexObj->Type() == ObjectType::ERROR_OBJ)
	{
		_results.push(indexObj);
		return;
	}

	_results.push(EvalIndexExpression(index->BaseToken, left, indexObj));
}

void RecursiveEvaluator::NodeEval(HashLiteral* hash)
{
	std::unordered_map<HashKey, HashEntry> elems;
	for (const auto& [key, value] : hash->Elements)
	{
		auto keyObj = Eval(key, _env);
		if (keyObj->Type() == ObjectType::ERROR_OBJ)
		{
			_results.push(keyObj);
			return;
		}

		auto valueObj = Eval(value, _env);
		if (valueObj->Type() == ObjectType::ERROR_OBJ)
		{
			_results.push(valueObj);
			return;
		}
		elems[HashKey{ keyObj->Type(), keyObj->Inspect() }] = HashEntry{ keyObj, valueObj };
	}
	_results.push(HashObj::New(elems));
}

void RecursiveEvaluator::NodeEval(WhileExpression* whileEx)
{
	auto condition = Eval(whileEx->Condition, _env);

	condition = UnwrapIfReturnObj(condition);
	condition = UnwrapIfIdentObj(condition);

	if (condition->Type() == ObjectType::ERROR_OBJ)
	{
		_results.push(condition);
		return;
	}

	auto booleanObj = EvalAsBoolean(whileEx->BaseToken, condition.get());

	if (booleanObj->Type() == ObjectType::ERROR_OBJ)
	{
		_results.push(booleanObj);
		return;
	}

	while (booleanObj == BooleanObj::TRUE_OBJ_REF)
	{
		auto evaluated = Eval(whileEx->Action, _env);
		if (evaluated != nullptr)
		{
			if (evaluated->Type() == ObjectType::RETURN_OBJ)
			{
				auto* ret = dynamic_no_copy_cast<ReturnObj>(evaluated);
				if (ret->Value == BreakObj::BREAK_OBJ_REF)
				{
					return;
				}

				if (ret->Value != ContinueObj::CONTINUE_OBJ_REF)
				{
					_results.push(evaluated);
					return;
				}
			}

			if (evaluated->Type() == ObjectType::ERROR_OBJ)
			{
				_results.push(evaluated);
				return;
			}
		}
		condition = Eval(whileEx->Condition, _env);

		condition = UnwrapIfReturnObj(condition);
		condition = UnwrapIfIdentObj(condition);

		if (condition->Type() == ObjectType::ERROR_OBJ)
		{
			_results.push(condition);
			return;
		}

		booleanObj = EvalAsBoolean(whileEx->BaseToken, condition.get());

		if (booleanObj->Type() == ObjectType::ERROR_OBJ)
		{
			_results.push(booleanObj);
			return;
		}
	}
	_results.push(NullObj::NULL_OBJ_REF);
}

void RecursiveEvaluator::NodeEval(ForExpression* forEx)
{
	auto init = Eval(forEx->Init, _env);

	auto condition = Eval(forEx->Condition, _env);

	condition = UnwrapIfReturnObj(condition);
	condition = UnwrapIfIdentObj(condition);

	if (condition->Type() == ObjectType::ERROR_OBJ)
	{
		_results.push(condition);
		return;
	}

	auto booleanObj = EvalAsBoolean(forEx->BaseToken, condition.get());

	if (booleanObj->Type() == ObjectType::ERROR_OBJ)
	{
		_results.push(booleanObj);
		return;
	}

	while (booleanObj == BooleanObj::TRUE_OBJ_REF)
	{
		auto evaluated = Eval(forEx->Action, _env);
		if (evaluated != nullptr)
		{
			if (evaluated->Type() == ObjectType::RETURN_OBJ)
			{
				auto* ret = dynamic_no_copy_cast<ReturnObj>(evaluated);
				if (ret->Value == BreakObj::BREAK_OBJ_REF)
				{
					return;
				}

				if (ret->Value != ContinueObj::CONTINUE_OBJ_REF)
				{
					_results.push(evaluated);
					return;
				}
			}

			if (evaluated->Type() == ObjectType::ERROR_OBJ)
			{
				_results.push(evaluated);
				return;
			}
		}

		auto post = Eval(forEx->Post, _env);

		condition = Eval(forEx->Condition, _env);

		condition = UnwrapIfReturnObj(condition);
		condition = UnwrapIfIdentObj(condition);

		if (condition->Type() == ObjectType::ERROR_OBJ)
		{
			_results.push(condition);
			return;
		}

		booleanObj = EvalAsBoolean(forEx->BaseToken, condition.get());

		if (booleanObj->Type() == ObjectType::ERROR_OBJ)
		{
			_results.push(booleanObj);
			return;
		}
	}
	_results.push(NullObj::NULL_OBJ_REF);
}

void RecursiveEvaluator::NodeEval(BreakStatement* breakStmt)
{
	_results.push(ReturnObj::New(BreakObj::BREAK_OBJ_REF));
}

void RecursiveEvaluator::NodeEval(ContinueStatement* continueStmt)
{
	_results.push(ReturnObj::New(ContinueObj::CONTINUE_OBJ_REF));
}

void RecursiveEvaluator::NodeEval(NullLiteral* null)
{
	_results.push(NullObj::NULL_OBJ_REF);
}

std::vector<std::shared_ptr<IObject>> RecursiveEvaluator::EvalExpressions(const std::vector<std::shared_ptr<IExpression>>& expressions)
{
	std::vector<std::shared_ptr<IObject>> result;
	for (const auto& expr : expressions)
	{
		auto evaluated = Eval(expr, _env);
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
std::shared_ptr<IObject> RecursiveEvaluator::ApplyFunction(const std::shared_ptr<FunctionObj>& func, const std::vector<std::shared_ptr<IObject>>& args)
{
	auto envHolder = _env; //save the current environment
	auto extEnv = ExtendFunctionEnv(_env, func, args);
	auto evaluated = Eval(func->Body, extEnv);
	_env = envHolder; //restore the environment
	return evaluated;
}