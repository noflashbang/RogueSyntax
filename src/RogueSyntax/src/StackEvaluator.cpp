#pragma once
#include "pch.h"
#include "StackEvaluator.h"


const IObject* StackEvaluator::Eval(const INode* node, const uint32_t env)
{
	Push_Eval(node, 0, env);

	uint32_t useEnv = env;

	while (!_stack.empty())
	{
		auto [currentNode, currentSignal, currentEnv] = _stack.top();
		_stack.pop();

		_currentSignal = currentSignal;
		_currentEnv = currentEnv;
		//check for errors
		if (ResultIsError())
		{
			return Pop_Result();
		}

		currentNode->Eval(this);
	}

	if (_results.empty())
	{
		return nullptr;
	}

	return Pop_Result();
}


void StackEvaluator::Push_Eval(const INode* node, const int32_t signal, const uint32_t env)
{
	_stack.emplace( node, signal, env );
}
void StackEvaluator::Pop_Eval()
{
	_stack.pop();
}
void StackEvaluator::Push_Result(const IObject* result)
{
	_results.emplace(result);
}
const IObject* StackEvaluator::Pop_Result()
{
	if (_results.empty())
	{
		return VoidObj::VOID_OBJ_REF;
	}
	auto result = _results.top();
	_results.pop();
	return result;
}

const IObject* StackEvaluator::Pop_ResultAndUnwrap()
{
	auto result = _results.top();
	_results.pop();

	result = UnwrapIfReturnObj(result);
	result = UnwrapIfIdentObj(result);

	return result;
}

bool StackEvaluator::HasResults() const
{
	return !_results.empty();
}

bool StackEvaluator::ResultIsError() const
{
	if (_results.empty())
	{
		return false;
	}

	return _results.top()->IsThisA<ErrorObj>();
}

bool StackEvaluator::ResultIsReturn() const
{
	if (_results.empty())
	{
		return false;
	}

	return _results.top()->IsThisA<ReturnObj>();
}

bool StackEvaluator::ResultIsIdent() const
{
	if (_results.empty())
	{
		return false;
	}

	return _results.top()->IsThisA<IdentifierObj>();
}

size_t StackEvaluator::ResultCount() const
{
	return _results.size();
}

void StackEvaluator::NodeEval(const Program* program)
{
	for (const auto& iter : std::views::reverse(program->Statements))
	{
		Push_Eval(iter, 0, _currentEnv);
	}
}
void StackEvaluator::NodeEval(const BlockStatement* block)
{
	if (ResultIsError() || ResultIsReturn())
	{
		return;
	}

	if (_currentSignal < block->Statements.size())
	{
		//get the statement for the current _currentSignal
		auto& stmt = block->Statements[_currentSignal];
		Push_Eval(block, _currentSignal + 1, _currentEnv);
		Push_Eval(stmt, 0, _currentEnv);
	}
}
void StackEvaluator::NodeEval(const ExpressionStatement* expression)
{
	Push_Eval(expression->Expression, 0, _currentEnv);
}
void StackEvaluator::NodeEval(const ReturnStatement* ret)
{
	if (_currentSignal == 0)
	{
		Push_Eval(ret, 1, _currentEnv);
		Push_Eval(ret->ReturnValue, 0, _currentEnv);
	}
	else
	{
		auto result = Pop_Result();
		Push_Result(EvalFactory->New<ReturnObj>(result));
	}
}
void StackEvaluator::NodeEval(const LetStatement* let)
{
	if(let->Name->IsThisA<Identifier>())
	{
		if (_currentSignal == 0)
		{
			Push_Eval(let, 1, _currentEnv);
			Push_Eval(let->Value, 0, _currentEnv);
			Push_Eval(let->Name, 0, _currentEnv);
		}
		else
		{
			if (ResultIsError())
			{
				return;
			}

			auto value = Pop_ResultAndUnwrap();

			if (ResultIsError())
			{
				return;
			}

			auto identResult = Pop_Result();
			identResult = UnwrapIfReturnObj(identResult);

			auto identClone = EvalFactory->Clone(identResult);
			auto* ident = dynamic_cast<IdentifierObj*>(identClone);
			auto result = ident->Set(nullptr, value);
			EvalEnvironment->Set(_currentEnv, ident->Name, value);
		}
	}
	else if (typeid(*(let->Name)) == typeid(IndexExpression))
	{
		if (_currentSignal == 0)
		{
			auto* index = dynamic_cast<const IndexExpression*>(let->Name);

			Push_Eval(let, 1, _currentEnv);
			Push_Eval( let->Value, 0, _currentEnv);
			Push_Eval( index->Left, 0, _currentEnv);
			Push_Eval( index->Index, 0, _currentEnv);
		}
		else
		{
			if (ResultIsError())
			{
				return;
			}
			auto value = Pop_ResultAndUnwrap();

			if (ResultIsError())
			{
				return;
			}
			auto left = Pop_ResultAndUnwrap();

			if (ResultIsError())
			{
				return;
			}
			auto index = Pop_ResultAndUnwrap();

			auto assignableClone = EvalFactory->Clone(left);
			auto* assignable = dynamic_cast<IAssignableObject*>(assignableClone);
			auto result = assignable->Set(index, value);
			//results.push(result);
		}
	}
	else
	{
		Push_Result(MakeError(_currentEnv, "let target must be identifier or index expression", let->BaseToken));
	}
}
void StackEvaluator::NodeEval(const Identifier* ident)
{
	if (EvalBuiltIn->IsBuiltIn(ident->Value))
	{
		Push_Result(EvalFactory->New<BuiltInObj>(ident->Value));
	}
	else
	{
		auto value = EvalEnvironment->Get(_currentEnv, ident->Value);
		if (value != nullptr)
		{
			Push_Result(EvalFactory->New<IdentifierObj>(ident->Value, value));
			//EvalEnvironment->Set(env, ident->Value, value);
		}
		else
		{
			Push_Result(EvalFactory->New<IdentifierObj>(ident->Value, NullObj::NULL_OBJ_REF));
			EvalEnvironment->Set(_currentEnv, ident->Value, NullObj::NULL_OBJ_REF);
		}
		//result = MakeError(std::format("identifier not found: {}", ident->Value), ident->BaseToken);
	}
}
void StackEvaluator::NodeEval(const IntegerLiteral* integer)
{
	Push_Result(EvalFactory->New<IntegerObj>(integer->Value));
}
void StackEvaluator::NodeEval(const BooleanLiteral* boolean)
{
	Push_Result(boolean->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF);
}
void StackEvaluator::NodeEval(const StringLiteral* string)
{
	Push_Result(EvalFactory->New<StringObj>(string->Value));
}
void StackEvaluator::NodeEval(const DecimalLiteral* decimal)
{
	Push_Result(EvalFactory->New<DecimalObj>(decimal->Value));
}
void StackEvaluator::NodeEval(const PrefixExpression* prefix)
{
	if (_currentSignal == 0)
	{
		Push_Eval(prefix, 1, _currentEnv);
		Push_Eval(prefix->Right, 0, _currentEnv);
	}
	else
	{
		auto lastResult = Pop_ResultAndUnwrap();
		Push_Result(EvalPrefixExpression(_currentEnv, prefix->BaseToken, lastResult));
	}
}
void StackEvaluator::NodeEval(const InfixExpression* infix)
{
	if (_currentSignal == 0)
	{
		Push_Eval(infix, 1, _currentEnv);
		Push_Eval(infix->Left, 0, _currentEnv);
	}
	else if (_currentSignal == 1)
	{
		Push_Eval(infix, 2, _currentEnv);
		Push_Eval(infix->Right, 0, _currentEnv);
	}
	else
	{
		if (ResultIsError())
		{
			return;
		}
		auto right = Pop_ResultAndUnwrap();

		if (ResultIsError())
		{
			return;
		}
		auto left = Pop_ResultAndUnwrap();

		auto result = EvalInfixExpression(_currentEnv, infix->BaseToken, left, right);
		Push_Result(result);
	}
}
void StackEvaluator::NodeEval(const IfStatement* ifExpr)
{
	if (_currentSignal == 0)
	{
		Push_Eval(ifExpr,  1, _currentEnv);
		Push_Eval(ifExpr->Condition, 0, _currentEnv);
	}
	else
	{
		auto lastResult = Pop_ResultAndUnwrap();

		auto evalBool = EvalAsBoolean(_currentEnv, ifExpr->BaseToken, lastResult);

		if (evalBool->IsThisA<ErrorObj>())
		{
			Push_Result(evalBool);
			return;
		}

		if (evalBool == BooleanObj::TRUE_OBJ_REF)
		{
			Push_Eval(ifExpr->Consequence, 0, _currentEnv);
		}
		else if (ifExpr->Alternative != nullptr)
		{
			Push_Eval(ifExpr->Alternative, 0, _currentEnv);
		}
		else
		{
			Push_Result(NullObj::NULL_OBJ_REF);
		}
	}
}
void StackEvaluator::NodeEval(const FunctionLiteral* function)
{
	Push_Result(EvalFactory->New<FunctionObj>(function->Parameters, function->Body));
}
void StackEvaluator::NodeEval(const CallExpression* call)
{
	if (_currentSignal == 0)
	{
		Push_Eval(call, 1, _currentEnv);
		Push_Eval(call->Function, 0, _currentEnv);
	}
	else if (_currentSignal == 1)
	{
		auto lastResult = Pop_Result();

		lastResult = UnwrapIfIdentObj(lastResult);

		if (!(lastResult->IsThisA<FunctionObj>() || lastResult->IsThisA<BuiltInObj>()))
		{
			Push_Result(MakeError(_currentEnv, std::format("literal not a function: {}", lastResult->Inspect()), call->BaseToken));
			return;
		}
		auto function = lastResult;
		Push_Result(function);

		Push_Eval(call, 2, _currentEnv);
		// Evaluate arguments
		for (const auto& iter : call->Arguments)
		{
			Push_Eval(iter, 0, _currentEnv);
		}
	}
	else if (_currentSignal == 2)
	{
		std::vector<const IObject*> evalArgs;
		if (ResultCount() < call->Arguments.size())
		{
			Push_Result(MakeError(_currentEnv, "not enough arguments", call->BaseToken));
			return;
		}

		for (size_t i = 0; i < call->Arguments.size(); i++)
		{
			if (ResultIsError())
			{
				return;
			}

			auto arg = Pop_ResultAndUnwrap();
			evalArgs.push_back(arg);
		}
		auto function = Pop_Result();

		if (function->IsThisA<BuiltInObj>())
		{
			auto func = dynamic_cast<const BuiltInObj*>(function);
			auto builtInToCall = EvalBuiltIn->GetBuiltInFunction(func->Name);
			if (builtInToCall == nullptr)
			{
				Push_Result(MakeError(_currentEnv, std::format("unknown function: {}", func->Name), call->BaseToken));
				return;
			}

			try
			{
				auto result = builtInToCall(evalArgs);
				if (result != VoidObj::VOID_OBJ_REF) //check for a "void" return
				{
					Push_Result(result);
				}
			}
			catch (const std::exception& e)
			{
				_results.push(MakeError(_currentEnv, e.what(), call->BaseToken));
			}
		}
		else
		{
			auto func = dynamic_cast<const FunctionObj*>(function);
			auto extEnv = ExtendFunctionEnv(_currentEnv, func, evalArgs);
			Push_Eval(call, 3, extEnv);
			Push_Eval(func->Body, 0, extEnv);
		}
	}
	else
	{
		//just releasing the environment
		EvalEnvironment->Release(_currentEnv);
	}
}
void StackEvaluator::NodeEval(const ArrayLiteral* array)
{
	std::vector<const IObject*> elements;

	if (_currentSignal == 0)
	{
		Push_Eval(array, 1, _currentEnv);

		for (const auto& elem : array->Elements)
		{
			Push_Eval(elem, 0, _currentEnv);
		}
	}
	else
	{
		std::vector<const IObject*> evalArgs;
		if (ResultCount() < array->Elements.size())
		{
			Push_Result(MakeError(_currentEnv, "failed to evaluate all array elements", array->BaseToken));
			return;
		}

		for (size_t i = 0; i < array->Elements.size(); i++)
		{
			if (ResultIsError())
			{
				return;
			}
			auto arg = Pop_ResultAndUnwrap();
			evalArgs.push_back(arg);
		}
		Push_Result(EvalFactory->New<ArrayObj>(evalArgs));
	}
}
void StackEvaluator::NodeEval(const IndexExpression* index)
{
	if (_currentSignal == 0)
	{
		Push_Eval(index, 1, _currentEnv);
		Push_Eval(index->Left, 0, _currentEnv);
		Push_Eval(index->Index, 0, _currentEnv);

	}
	else
	{
		auto left = Pop_Result();
		auto indexObj = Pop_Result();

		left = UnwrapIfIdentObj(left);
		indexObj = UnwrapIfIdentObj(indexObj);

		auto result = EvalIndexExpression(_currentEnv, index->BaseToken, left, indexObj);
		Push_Result(result);
	}
}
void StackEvaluator::NodeEval(const HashLiteral* hash)
{
	std::vector<const IObject*> elements;

	if (_currentSignal == 0)
	{
		Push_Eval(hash, 1, _currentEnv);
		for (const auto& [key, value] : hash->Elements)
		{
			Push_Eval(key, 0, _currentEnv);
			Push_Eval(value, 0, _currentEnv);
		}
	}
	else
	{
		std::unordered_map<HashKey, HashEntry> evalArgs;
		if (ResultCount() < hash->Elements.size())
		{
			Push_Result(MakeError(_currentEnv, "failed to evaluate all hash elements", hash->BaseToken));
			return;
		}

		for (size_t i = 0; i < hash->Elements.size(); i++)
		{
			if(ResultIsError())
			{
				return;
			}

			auto key = Pop_ResultAndUnwrap();

			if (ResultIsError())
			{
				return;
			}

			auto value = Pop_ResultAndUnwrap();

			evalArgs[HashKey{ key->Type(), key->Inspect() }] = HashEntry{ key, value };
		}
		Push_Result(EvalFactory->New<HashObj>(evalArgs));
	}
}
void StackEvaluator::NodeEval(const NullLiteral* null)
{
	Push_Result(NullObj::NULL_OBJ_REF);
}
void StackEvaluator::NodeEval(const WhileStatement* whileExp)
{
	if (_currentSignal == 0)
	{
		//check the action for return/error
		if (ResultIsReturn())
		{
			auto top = Pop_Result();

			auto* ret = dynamic_cast<const ReturnObj*>(top);
			if (!(ret->Value == ContinueObj::CONTINUE_OBJ_REF || ret->Value == BreakObj::BREAK_OBJ_REF))
			{
				Push_Result(top);
				return;
			}
			if (ret->Value == BreakObj::BREAK_OBJ_REF)
			{
				return;
			}
		}

		if (ResultIsError())
		{
			return;
		}

		Push_Eval(whileExp, 1, _currentEnv);
		Push_Eval(whileExp->Condition, 0, _currentEnv);
	}
	else
	{
		auto lastResult = Pop_ResultAndUnwrap();

		auto booleanObj = EvalAsBoolean(_currentEnv, whileExp->BaseToken, lastResult);

		if (booleanObj->IsThisA<ErrorObj>())
		{
			Push_Result(booleanObj);
			return;
		}

		if (booleanObj == BooleanObj::TRUE_OBJ_REF)
		{
			Push_Eval(whileExp, 0, _currentEnv);
			Push_Eval(whileExp->Action, 0, _currentEnv);
		}
	}
}
void StackEvaluator::NodeEval(const ForStatement* forExp)
{
	if (_currentSignal == 0)
	{
		Push_Eval(forExp, 1, _currentEnv);
		Push_Eval(forExp->Init, 0, _currentEnv);
	}
	else if (_currentSignal == 1)
	{
		//condition
		Push_Eval(forExp, 2, _currentEnv);
		Push_Eval(forExp->Condition, 0, _currentEnv);
	}
	else if (_currentSignal == 2)
	{
		auto lastResult = Pop_ResultAndUnwrap();

		auto booleanObj = EvalAsBoolean(_currentEnv, forExp->BaseToken, lastResult);

		if (booleanObj->IsThisA<ErrorObj>())
		{
			Push_Result(booleanObj);
			return;
		}

		if (booleanObj == BooleanObj::TRUE_OBJ_REF)
		{
			Push_Eval(forExp, 3, _currentEnv);
			Push_Eval(forExp->Action, 0, _currentEnv);
		}
	}
	else if (_currentSignal == 3)
	{
		//check the action for return/error
		if (ResultIsReturn())
		{
			auto top = Pop_Result();

			auto* ret = dynamic_cast<const ReturnObj*>(top);
			if (!(ret->Value == ContinueObj::CONTINUE_OBJ_REF || ret->Value == BreakObj::BREAK_OBJ_REF))
			{
				Push_Result(top);
				return;
			}
			if (ret->Value == BreakObj::BREAK_OBJ_REF)
			{
				return;
			}
		}

		if (ResultIsError())
		{
			return;
		}
		Push_Eval(forExp, 1, _currentEnv);
		Push_Eval(forExp->Post, 0, _currentEnv);
	}
}
void StackEvaluator::NodeEval(const ContinueStatement* cont)
{
	Push_Result(EvalFactory->New<ReturnObj>(ContinueObj::CONTINUE_OBJ_REF));
}
void StackEvaluator::NodeEval(const BreakStatement* brk)
{
	Push_Result(EvalFactory->New<ReturnObj>(BreakObj::BREAK_OBJ_REF));
}