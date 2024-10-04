#pragma once
#include "pch.h"
#include "RecursiveEvaluator.h"


const IObject* RecursiveEvaluator::Eval(const INode* node, const uint32_t env)
{
	_env = env; // set the environment
	auto result = Eval(node);

	if (VoidObj::VOID_OBJ_REF == result)
	{
		return nullptr;
	}

	return result;
}

const IObject* RecursiveEvaluator::Eval(const INode* node)
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

void RecursiveEvaluator::NodeEval(const Program* program)
{
	_results.push(Eval(program));
}

void RecursiveEvaluator::NodeEval(const BlockStatement* block)
{
	for (const auto& stmt : block->Statements)
	{
		auto result = Eval(stmt, _env);
		_results.push(result);
		if (result == nullptr)
		{
			continue;
		}

		if (result->IsThisA<ReturnObj>() || result->IsThisA<ErrorObj>())
		{
			break;
		}
	}
}

void RecursiveEvaluator::NodeEval(const ExpressionStatement* expression)
{
	_results.push(Eval(expression->Expression, _env));
}

void RecursiveEvaluator::NodeEval(const ReturnStatement* ret)
{
	auto& store = EvalEnvironment->GetObjectStore(_env);
	auto value = Eval(ret->ReturnValue, _env);
	_results.push(store.New_ReturnObj(value));
}

void RecursiveEvaluator::NodeEval(const LetStatement* let)
{
	auto& store = EvalEnvironment->GetObjectStore(_env);
	auto value = Eval(let->Value, _env);

	value = UnwrapIfReturnObj(value);
	value = UnwrapIfIdentObj(value);

	if (value->IsThisA<ErrorObj>())
	{
		_results.push(value);
		return;
	}

	if (let->Name->IsThisA<Identifier>())
	{
		auto target = Eval(let->Name, _env);
		if (target->IsThisA<ErrorObj>())
		{
			_results.push(target);
			return;
		}
		
		auto identClone = target->Clone();
		store.Add(identClone);

		auto* ident = dynamic_cast<IdentifierObj*>(identClone);
		ident->Set(nullptr, value);
		EvalEnvironment->Set(_env, ident->Name, value);
	}
	else if (let->Name->IsThisA<IndexExpression>())
	{
		auto* index = dynamic_cast<const IndexExpression*>(let->Name);
		auto left = Eval(index->Left, _env);
		auto indexObj = Eval(index->Index, _env);

		left = UnwrapIfReturnObj(left);
		left = UnwrapIfIdentObj(left);

		if (left->IsThisA<ErrorObj>())
		{
			_results.push(left);
			return;
		}

		indexObj = UnwrapIfReturnObj(indexObj);
		indexObj = UnwrapIfIdentObj(indexObj);

		if (indexObj->IsThisA<ErrorObj>())
		{
			_results.push(indexObj);
			return;
		}

		auto assignableClone = left->Clone();
		store.Add(assignableClone);
		auto* assignable = dynamic_cast<IAssignableObject*>(assignableClone);
		if (assignable == nullptr)
		{
			_results.push(MakeError(_env, "left side of assignment is not assignable", index->BaseToken));
			return;
		}
		assignable->Set(indexObj, value);
	}
	else
	{
		_results.push(MakeError(_env, "let target must be identifier or index expression", let->BaseToken));
	}
}

void RecursiveEvaluator::NodeEval(const Identifier* ident)
{
	auto& store = EvalEnvironment->GetObjectStore(_env);
	if (EvalBuiltIn->IsBuiltIn(ident->Value))
	{
		_results.push(store.New_BuiltInObj(ident->Value));
	}
	else
	{
		auto value = EvalEnvironment->Get(_env, ident->Value);
		if (value != nullptr)
		{
			_results.push(store.New_IdentifierObj(ident->Value, value));
		}
		else
		{
			_results.push(store.New_IdentifierObj(ident->Value, NullObj::NULL_OBJ_REF));
		}
	}
}

void RecursiveEvaluator::NodeEval(const IntegerLiteral* integer)
{
	auto& store = EvalEnvironment->GetObjectStore(_env);
	_results.push(store.New_IntegerObj(integer->Value));
}

void RecursiveEvaluator::NodeEval(const BooleanLiteral* boolean)
{
	_results.push(boolean->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF);
}

void RecursiveEvaluator::NodeEval(const StringLiteral* string)
{
	auto& store = EvalEnvironment->GetObjectStore(_env);
	_results.push(store.New_StringObj(string->Value));
}

void RecursiveEvaluator::NodeEval(const DecimalLiteral* decimal)
{
	auto& store = EvalEnvironment->GetObjectStore(_env);
	_results.push(store.New_DecimalObj(decimal->Value));
}

void RecursiveEvaluator::NodeEval(const PrefixExpression* prefix)
{
	auto right = Eval(prefix->Right, _env);

	right = UnwrapIfReturnObj(right);
	right = UnwrapIfIdentObj(right);

	if (right->IsThisA<ErrorObj>())
	{
		_results.push(right);
		return;
	}
	_results.push(EvalPrefixExpression(_env, prefix->BaseToken, right));
}

void RecursiveEvaluator::NodeEval(const InfixExpression* infix)
{
	auto left = Eval(infix->Left, _env);

	left = UnwrapIfReturnObj(left);
	left = UnwrapIfIdentObj(left);

	if (left->IsThisA<ErrorObj>())
	{
		_results.push(left);
		return;
	}
	auto right = Eval(infix->Right, _env);

	right = UnwrapIfReturnObj(right);
	right = UnwrapIfIdentObj(right);

	if (right->IsThisA<ErrorObj>())
	{
		_results.push(right);
		return;
	}
	_results.push(EvalInfixExpression(_env, infix->BaseToken, left, right));
}

void RecursiveEvaluator::NodeEval(const IfStatement* ifExpr)
{
	auto condition = Eval(ifExpr->Condition, _env);

	condition = UnwrapIfReturnObj(condition);
	condition = UnwrapIfIdentObj(condition);

	if (condition->IsThisA<ErrorObj>())
	{
		_results.push(condition);
		return;
	}

	auto booleanObj = EvalAsBoolean(_env, ifExpr->BaseToken, condition);

	if (booleanObj->IsThisA<ErrorObj>())
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

void RecursiveEvaluator::NodeEval(const FunctionLiteral* function)
{
	auto& store = EvalEnvironment->GetObjectStore(_env);
	_results.push(store.New_FunctionObj(function->Parameters, function->Body));
}

void RecursiveEvaluator::NodeEval(const CallExpression* call)
{
	auto function = Eval(call->Function, _env);
	if (function->IsThisA<ErrorObj>())
	{
		_results.push(function);
		return;
	}

	function = UnwrapIfIdentObj(function);

	if (!(function->IsThisA<FunctionObj>() || function->IsThisA<BuiltInObj>()))
	{
		_results.push(MakeError(_env, std::format("literal not a function or builtin: {}", function->Inspect()), call->BaseToken));
		return;
	}

	auto evalArgs = EvalExpressions(call->Arguments);
	if (evalArgs.size() == 1 && evalArgs[0]->IsThisA<ErrorObj>())
	{
		_results.push(evalArgs[0]);
		return;
	}

	if (function->IsThisA<BuiltInObj>())
	{
		auto* builtInObj = dynamic_cast<const BuiltInObj*>(function);
		auto builtInToCall = EvalBuiltIn->GetBuiltInFunction(builtInObj->Name);

		if (builtInToCall == nullptr)
		{
			_results.push(MakeError(_env, std::format("builtin function not found: {}", builtInObj->Name), call->BaseToken));
			return;
		}

		try
		{
			auto result = builtInToCall(evalArgs);
			if (result != VoidObj::VOID_OBJ_REF) //check for a "void" return
			{
				_results.push(result);
			}
		}
		catch (const std::exception& e)
		{
			_results.push(MakeError(_env, e.what(), call->BaseToken));
		}
	}
	else
	{
		auto func = dynamic_cast<const FunctionObj*>(function);
		_results.push(ApplyFunction(func, evalArgs));
	}
}

void RecursiveEvaluator::NodeEval(const ArrayLiteral* array)
{
	auto& store = EvalEnvironment->GetObjectStore(_env);
	std::vector<const IObject*> elements;
	for (const auto& elem : array->Elements)
	{
		auto eval = Eval(elem, _env);
		if (eval->IsThisA<ErrorObj>())
		{
			_results.push(eval);
			return;
		}
		elements.push_back(eval);
	}
	_results.push(store.New_ArrayObj(elements));
}

void RecursiveEvaluator::NodeEval(const IndexExpression* index)
{
	auto left = Eval(index->Left, _env);
	auto indexObj = Eval(index->Index, _env);

	left = UnwrapIfReturnObj(left);
	left = UnwrapIfIdentObj(left);

	if (left->IsThisA<ErrorObj>())
	{
		_results.push(left);
		return;
	}

	indexObj = UnwrapIfReturnObj(indexObj);
	indexObj = UnwrapIfIdentObj(indexObj);

	if (indexObj->IsThisA<ErrorObj>())
	{
		_results.push(indexObj);
		return;
	}

	_results.push(EvalIndexExpression(_env, index->BaseToken, left, indexObj));
}

void RecursiveEvaluator::NodeEval(const HashLiteral* hash)
{
	auto& store = EvalEnvironment->GetObjectStore(_env);
	std::unordered_map<HashKey, HashEntry> elems;
	for (const auto& [key, value] : hash->Elements)
	{
		auto keyObj = Eval(key, _env);
		if (keyObj->IsThisA<ErrorObj>())
		{
			_results.push(keyObj);
			return;
		}

		auto valueObj = Eval(value, _env);
		if (valueObj->IsThisA<ErrorObj>())
		{
			_results.push(valueObj);
			return;
		}
		elems[HashKey{ keyObj->Type(), keyObj->Inspect() }] = HashEntry{ keyObj, valueObj };
	}
	_results.push(store.New_HashObj(elems));
}

void RecursiveEvaluator::NodeEval(const WhileStatement* whileEx)
{
	auto condition = Eval(whileEx->Condition, _env);

	condition = UnwrapIfReturnObj(condition);
	condition = UnwrapIfIdentObj(condition);

	if (condition->IsThisA<ErrorObj>())
	{
		_results.push(condition);
		return;
	}

	auto booleanObj = EvalAsBoolean(_env, whileEx->BaseToken, condition);

	if (booleanObj->IsThisA<ErrorObj>())
	{
		_results.push(booleanObj);
		return;
	}

	while (booleanObj == BooleanObj::TRUE_OBJ_REF)
	{
		auto evaluated = Eval(whileEx->Action, _env);
		if (evaluated != nullptr)
		{
			if (evaluated->IsThisA<ReturnObj>())
			{
				auto* ret = dynamic_cast<const ReturnObj*>(evaluated);
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

			if (evaluated->IsThisA<ErrorObj>())
			{
				_results.push(evaluated);
				return;
			}
		}
		condition = Eval(whileEx->Condition, _env);

		condition = UnwrapIfReturnObj(condition);
		condition = UnwrapIfIdentObj(condition);

		if (condition->IsThisA<ErrorObj>())
		{
			_results.push(condition);
			return;
		}

		booleanObj = EvalAsBoolean(_env, whileEx->BaseToken, condition);

		if (booleanObj->IsThisA<ErrorObj>())
		{
			_results.push(booleanObj);
			return;
		}
	}
	_results.push(NullObj::NULL_OBJ_REF);
}

void RecursiveEvaluator::NodeEval(const ForStatement* forEx)
{
	auto init = Eval(forEx->Init, _env);

	auto condition = Eval(forEx->Condition, _env);

	condition = UnwrapIfReturnObj(condition);
	condition = UnwrapIfIdentObj(condition);

	if (condition->IsThisA<ErrorObj>())
	{
		_results.push(condition);
		return;
	}

	auto booleanObj = EvalAsBoolean(_env, forEx->BaseToken, condition);

	if (booleanObj->IsThisA<ErrorObj>())
	{
		_results.push(booleanObj);
		return;
	}

	while (booleanObj == BooleanObj::TRUE_OBJ_REF)
	{
		auto evaluated = Eval(forEx->Action, _env);
		if (evaluated != nullptr)
		{
			if (evaluated->IsThisA<ReturnObj>())
			{
				auto* ret = dynamic_cast<const ReturnObj*>(evaluated);
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

			if (evaluated->IsThisA<ErrorObj>())
			{
				_results.push(evaluated);
				return;
			}
		}

		auto post = Eval(forEx->Post, _env);

		condition = Eval(forEx->Condition, _env);

		condition = UnwrapIfReturnObj(condition);
		condition = UnwrapIfIdentObj(condition);

		if (condition->IsThisA<ErrorObj>())
		{
			_results.push(condition);
			return;
		}

		booleanObj = EvalAsBoolean(_env, forEx->BaseToken, condition);

		if (booleanObj->IsThisA<ErrorObj>())
		{
			_results.push(booleanObj);
			return;
		}
	}
	_results.push(NullObj::NULL_OBJ_REF);
}

void RecursiveEvaluator::NodeEval(const BreakStatement* breakStmt)
{
	auto& store = EvalEnvironment->GetObjectStore(_env);
	_results.push(store.New_ReturnObj(BreakObj::BREAK_OBJ_REF));
}

void RecursiveEvaluator::NodeEval(const ContinueStatement* continueStmt)
{
	auto& store = EvalEnvironment->GetObjectStore(_env);
	_results.push(store.New_ReturnObj(ContinueObj::CONTINUE_OBJ_REF));
}

void RecursiveEvaluator::NodeEval(const NullLiteral* null)
{
	_results.push(NullObj::NULL_OBJ_REF);
}

std::vector<const IObject*> RecursiveEvaluator::EvalExpressions(const std::vector<IExpression*>& expressions)
{
	std::vector<const IObject*> result;
	for (const auto& expr : expressions)
	{
		auto evaluated = Eval(expr, _env);
		if (evaluated->IsThisA<ErrorObj>())
		{
			return { evaluated };
		}

		if (evaluated->IsThisA<ReturnObj>())
		{
			auto ret = dynamic_cast<const ReturnObj*>(evaluated);

			auto unwrapped = UnwrapIfReturnObj(ret->Value);
			unwrapped = UnwrapIfIdentObj(unwrapped);

			result.push_back(unwrapped);
		}
		else if (evaluated->IsThisA<IdentifierObj>())
		{
			auto ident = dynamic_cast<const IdentifierObj*>(evaluated);
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
const IObject* RecursiveEvaluator::ApplyFunction(const FunctionObj* func, const std::vector<const IObject*>& args)
{
	auto envHolder = _env; //save the current environment
	auto extEnv = ExtendFunctionEnv(_env, func, args);
	auto evaluated = Eval(func->Body, extEnv);
	EvalEnvironment->Release(extEnv);
	_env = envHolder; //restore the environment
	return evaluated;
}