#pragma once
#include "pch.h"
#include "StackEvaluator.h"


std::shared_ptr<IObject> StackEvaluator::Eval(const std::shared_ptr<INode>& node, const uint32_t env)
{
	Push_Eval(node.get(), 0, env);

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


void StackEvaluator::Push_Eval(INode* node, const int32_t signal, const uint32_t env)
{
	_stack.emplace( node, signal, env );
}
void StackEvaluator::Pop_Eval()
{
	_stack.pop();
}
void StackEvaluator::Push_Result(std::shared_ptr<IObject> result)
{
	_results.emplace(result);
}
std::shared_ptr<IObject> StackEvaluator::Pop_Result()
{
	if (_results.empty())
	{
		return VoidObj::VOID_OBJ_REF;
	}
	auto result = _results.top();
	_results.pop();
	return result;
}

std::shared_ptr<IObject> StackEvaluator::Pop_ResultAndUnwrap()
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

	return _results.top()->Type() == ObjectType::ERROR_OBJ;
}

bool StackEvaluator::ResultIsReturn() const
{
	if (_results.empty())
	{
		return false;
	}

	return _results.top()->Type() == ObjectType::RETURN_OBJ;
}

bool StackEvaluator::ResultIsIdent() const
{
	if (_results.empty())
	{
		return false;
	}

	return _results.top()->Type() == ObjectType::IDENT_OBJ;
}

size_t StackEvaluator::ResultCount() const
{
	return _results.size();
}

void StackEvaluator::NodeEval(Program* program)
{
	for (const auto& iter : std::views::reverse(program->Statements))
	{
		Push_Eval(iter.get(), 0, _currentEnv);
	}
}
void StackEvaluator::NodeEval(BlockStatement* block)
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
		Push_Eval(stmt.get(), 0, _currentEnv);
	}
}
void StackEvaluator::NodeEval(ExpressionStatement* expression)
{
	Push_Eval(expression->Expression.get(), 0, _currentEnv);
}
void StackEvaluator::NodeEval(ReturnStatement* ret)
{
	if (_currentSignal == 0)
	{
		Push_Eval(ret, 1, _currentEnv);
		Push_Eval(ret->ReturnValue.get(), 0, _currentEnv);
	}
	else
	{
		auto result = Pop_Result();
		Push_Result(ReturnObj::New(result));
	}
}
void StackEvaluator::NodeEval(LetStatement* let)
{
	if (typeid(*(let->Name.get())) == typeid(Identifier))
	{
		if (_currentSignal == 0)
		{
			Push_Eval(let, 1, _currentEnv);
			Push_Eval(let->Value.get(), 0, _currentEnv);
			Push_Eval(let->Name.get(), 0, _currentEnv);
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

			auto* ident = dynamic_no_copy_cast<IdentifierObj>(identResult);
			auto result = ident->Set(nullptr, value);
			EvalEnvironment->Set(_currentEnv, ident->Name, value);
			//results.push(result);
		}
	}
	else if (typeid(*(let->Name.get())) == typeid(IndexExpression))
	{
		if (_currentSignal == 0)
		{
			auto* index = dynamic_no_copy_cast<IndexExpression>(let->Name);

			Push_Eval(let, 1, _currentEnv);
			Push_Eval( let->Value.get(), 0, _currentEnv);
			Push_Eval( index->Left.get(), 0, _currentEnv);
			Push_Eval( index->Index.get(), 0, _currentEnv);
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

			auto* assignable = dynamic_no_copy_cast<IAssignableObject>(left);
			auto result = assignable->Set(index, value);
			//results.push(result);
		}
	}
	else
	{
		Push_Result(MakeError("let target must be identifier or index expression", let->BaseToken));
	}
}
void StackEvaluator::NodeEval(Identifier* ident)
{
	if (EvalBuiltIn->IsBuiltIn(ident->Value))
	{
		Push_Result(BuiltInObj::New(ident->Value));
	}
	else
	{
		auto value = EvalEnvironment->Get(_currentEnv, ident->Value);
		if (value != nullptr)
		{
			Push_Result(IdentifierObj::New(ident->Value, value));
			//EvalEnvironment->Set(env, ident->Value, value);
		}
		else
		{
			Push_Result(IdentifierObj::New(ident->Value, NullObj::NULL_OBJ_REF));
			EvalEnvironment->Set(_currentEnv, ident->Value, NullObj::NULL_OBJ_REF);
		}
		//result = MakeError(std::format("identifier not found: {}", ident->Value), ident->BaseToken);
	}
}
void StackEvaluator::NodeEval(IntegerLiteral* integer)
{
	Push_Result(IntegerObj::New(integer->Value));
}
void StackEvaluator::NodeEval(BooleanLiteral* boolean)
{
	Push_Result(boolean->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF);
}
void StackEvaluator::NodeEval(StringLiteral* string)
{
	Push_Result(StringObj::New(string->Value));
}
void StackEvaluator::NodeEval(DecimalLiteral* decimal)
{
	Push_Result(DecimalObj::New(decimal->Value));
}
void StackEvaluator::NodeEval(PrefixExpression* prefix)
{
	if (_currentSignal == 0)
	{
		Push_Eval(prefix, 1, _currentEnv);
		Push_Eval(prefix->Right.get(), 0, _currentEnv);
	}
	else
	{
		auto lastResult = Pop_ResultAndUnwrap();
		Push_Result(EvalPrefixExpression(prefix->BaseToken, lastResult));
	}
}
void StackEvaluator::NodeEval(InfixExpression* infix)
{
	if (_currentSignal == 0)
	{
		Push_Eval(infix, 1, _currentEnv);
		Push_Eval(infix->Left.get(), 0, _currentEnv);
	}
	else if (_currentSignal == 1)
	{
		Push_Eval(infix, 2, _currentEnv);
		Push_Eval(infix->Right.get(), 0, _currentEnv);
	}
	else
	{
		//if (results.size() < 2)
		//{
		//	results.push(MakeError("infix expression requires two operands", infix->BaseToken));
		//	break;
		//}

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

		auto result = EvalInfixExpression(infix->BaseToken, left, right);
		Push_Result(result);
	}
}
void StackEvaluator::NodeEval(IfStatement* ifExpr)
{
	if (_currentSignal == 0)
	{
		Push_Eval(ifExpr,  1, _currentEnv);
		Push_Eval(ifExpr->Condition.get(), 0, _currentEnv);
	}
	else
	{
		auto lastResult = Pop_ResultAndUnwrap();

		auto evalBool = EvalAsBoolean(ifExpr->BaseToken, lastResult.get());

		if (evalBool->Type() == ObjectType::ERROR_OBJ)
		{
			Push_Result(evalBool);
			return;
		}

		if (evalBool == BooleanObj::TRUE_OBJ_REF)
		{
			Push_Eval(ifExpr->Consequence.get(), 0, _currentEnv);
		}
		else if (ifExpr->Alternative != nullptr)
		{
			Push_Eval(ifExpr->Alternative.get(), 0, _currentEnv);
		}
		else
		{
			Push_Result(NullObj::NULL_OBJ_REF);
		}
	}
}
void StackEvaluator::NodeEval(FunctionLiteral* function)
{

	Push_Result(FunctionObj::New(function->Parameters, function->Body));

}
void StackEvaluator::NodeEval(CallExpression* call)
{
	if (_currentSignal == 0)
	{
		Push_Eval(call, 1, _currentEnv);
		Push_Eval(call->Function.get(), 0, _currentEnv);
	}
	else if (_currentSignal == 1)
	{
		auto lastResult = Pop_Result();

		lastResult = UnwrapIfIdentObj(lastResult);

		if (lastResult->Type() != ObjectType::FUNCTION_OBJ && lastResult->Type() != ObjectType::BUILTIN_OBJ)
		{
			Push_Result(MakeError(std::format("literal not a function: {}", lastResult->Inspect()), call->BaseToken));
			return;
		}
		auto function = lastResult;
		Push_Result(function);

		Push_Eval(call, 2, _currentEnv);
		// Evaluate arguments
		for (const auto& iter : call->Arguments)
		{
			Push_Eval(iter.get(), 0, _currentEnv);
		}
	}
	else if (_currentSignal == 2)
	{
		std::vector<std::shared_ptr<IObject>> evalArgs;
		if (ResultCount() < call->Arguments.size())
		{
			Push_Result(MakeError("not enough arguments", call->BaseToken));
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

		if (function->Type() == ObjectType::BUILTIN_OBJ)
		{
			auto func = std::dynamic_pointer_cast<BuiltInObj>(function);
			auto builtInToCall = EvalBuiltIn->GetBuiltInFunction(func->Name);
			if (builtInToCall == nullptr)
			{
				Push_Result(MakeError(std::format("unknown function: {}", func->Name), call->BaseToken));
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
				_results.push(MakeError(e.what(), call->BaseToken));
			}
		}
		else
		{
			auto func = std::dynamic_pointer_cast<FunctionObj>(function);
			auto extEnv = ExtendFunctionEnv(_currentEnv, func, evalArgs);
			Push_Eval(call, 3, extEnv);
			Push_Eval(func->Body.get(), 0, extEnv);
		}
	}
	else
	{
		//just releasing the environment
		EvalEnvironment->Release(_currentEnv);
	}
}
void StackEvaluator::NodeEval(ArrayLiteral* array)
{
	std::vector<std::shared_ptr<IObject>> elements;

	if (_currentSignal == 0)
	{
		Push_Eval(array, 1, _currentEnv);

		for (const auto& elem : array->Elements)
		{
			Push_Eval(elem.get(), 0, _currentEnv);
		}
	}
	else
	{
		std::vector<std::shared_ptr<IObject>> evalArgs;
		if (ResultCount() < array->Elements.size())
		{
			Push_Result(MakeError("failed to evaluate all array elements", array->BaseToken));
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
		Push_Result(ArrayObj::New(evalArgs));
	}
}
void StackEvaluator::NodeEval(IndexExpression* index)
{
	if (_currentSignal == 0)
	{
		Push_Eval(index, 1, _currentEnv);
		Push_Eval(index->Left.get(), 0, _currentEnv);
		Push_Eval(index->Index.get(), 0, _currentEnv);

	}
	else
	{
		auto left = Pop_Result();
		auto indexObj = Pop_Result();

		left = UnwrapIfIdentObj(left);
		indexObj = UnwrapIfIdentObj(indexObj);

		auto result = EvalIndexExpression(index->BaseToken, left, indexObj);
		Push_Result(result);
	}
}
void StackEvaluator::NodeEval(HashLiteral* hash)
{
	std::vector<std::shared_ptr<IObject>> elements;

	if (_currentSignal == 0)
	{
		Push_Eval(hash, 1, _currentEnv);
		for (const auto& [key, value] : hash->Elements)
		{
			Push_Eval(key.get(), 0, _currentEnv);
			Push_Eval(value.get(), 0, _currentEnv);
		}
	}
	else
	{
		std::unordered_map<HashKey, HashEntry> evalArgs;
		if (ResultCount() < hash->Elements.size())
		{
			Push_Result(MakeError("failed to evaluate all hash elements", hash->BaseToken));
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
		Push_Result(HashObj::New(evalArgs));
	}
}
void StackEvaluator::NodeEval(NullLiteral* null)
{
	Push_Result(NullObj::NULL_OBJ_REF);
}
void StackEvaluator::NodeEval(WhileStatement* whileExp)
{
	if (_currentSignal == 0)
	{
		//check the action for return/error
		if (ResultIsReturn())
		{
			auto top = Pop_Result();

			auto* ret = dynamic_no_copy_cast<ReturnObj>(top);
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
		Push_Eval(whileExp->Condition.get(), 0, _currentEnv);
	}
	else
	{
		auto lastResult = Pop_ResultAndUnwrap();

		auto booleanObj = EvalAsBoolean(whileExp->BaseToken, lastResult.get());

		if (booleanObj->Type() == ObjectType::ERROR_OBJ)
		{
			Push_Result(booleanObj);
			return;
		}

		if (booleanObj == BooleanObj::TRUE_OBJ_REF)
		{
			Push_Eval(whileExp, 0, _currentEnv);
			Push_Eval(whileExp->Action.get(), 0, _currentEnv);
		}
	}
}
void StackEvaluator::NodeEval(ForStatement* forExp)
{
	if (_currentSignal == 0)
	{
		Push_Eval(forExp, 1, _currentEnv);
		Push_Eval(forExp->Init.get(), 0, _currentEnv);
	}
	else if (_currentSignal == 1)
	{
		//condition
		Push_Eval(forExp, 2, _currentEnv);
		Push_Eval(forExp->Condition.get(), 0, _currentEnv);
	}
	else if (_currentSignal == 2)
	{
		auto lastResult = Pop_ResultAndUnwrap();

		auto booleanObj = EvalAsBoolean(forExp->BaseToken, lastResult.get());

		if (booleanObj->Type() == ObjectType::ERROR_OBJ)
		{
			Push_Result(booleanObj);
			return;
		}

		if (booleanObj == BooleanObj::TRUE_OBJ_REF)
		{
			Push_Eval(forExp, 3, _currentEnv);
			Push_Eval(forExp->Action.get(), 0, _currentEnv);
		}
	}
	else if (_currentSignal == 3)
	{
		//check the action for return/error
		if (ResultIsReturn())
		{
			auto top = Pop_Result();

			auto* ret = dynamic_no_copy_cast<ReturnObj>(top);
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
		Push_Eval(forExp->Post.get(), 0, _currentEnv);
	}
}
void StackEvaluator::NodeEval(ContinueStatement* cont)
{
	Push_Result(ReturnObj::New(ContinueObj::CONTINUE_OBJ_REF));
}
void StackEvaluator::NodeEval(BreakStatement* brk)
{
	Push_Result(ReturnObj::New(BreakObj::BREAK_OBJ_REF));
}