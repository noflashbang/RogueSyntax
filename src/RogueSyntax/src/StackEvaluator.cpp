#pragma once
#include "pch.h"
#include "StackEvaluator.h"


std::shared_ptr<IObject> StackEvaluator::Eval(const std::shared_ptr<INode>& node, const std::shared_ptr<Environment>& env, const std::shared_ptr<BuiltIn>& builtIn)
{
	std::stack<std::tuple<const INode*, int32_t, std::shared_ptr<Environment>>> stack;
	std::stack<std::shared_ptr<IObject>> results;

	stack.push({ node.get(), 0, nullptr });

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
			Handler_Program(program, signal, useEnv.get());
			break;
		}
		case NodeType::ExpressionStatement:
		{
			const auto* expression = dynamic_cast<const ExpressionStatement*>(currentNode);
			Handler_ExpressionStatement(expression, signal, useEnv.get());
			break;
		}
		case NodeType::NullLiteral:
		{
			const auto* null = dynamic_cast<const NullLiteral*>(currentNode);
			Handler_NullLiteral(null, signal, useEnv.get());
			break;
		}
		case NodeType::IntegerLiteral:
		{
			const auto* integer = dynamic_cast<const IntegerLiteral*>(currentNode);
			Handler_IntegerLiteral(integer, signal, useEnv.get());
			break;
		}
		case NodeType::DecimalLiteral:
		{
			const auto* decimal = dynamic_cast<const DecimalLiteral*>(currentNode);
			Handler_DecimalLiteral(decimal, signal, useEnv.get());
			break;
		}
		case NodeType::StringLiteral:
		{
			const auto* string = dynamic_cast<const StringLiteral*>(currentNode);
			Handler_StringLiteral(string, signal, useEnv.get());
			break;
		}
		case NodeType::BooleanLiteral:
		{
			const auto* boolean = dynamic_cast<const BooleanLiteral*>(currentNode);
			Handler_BooleanLiteral(boolean, signal, useEnv.get());
			break;
		}
		case NodeType::ArrayLiteral:
		{
			const auto* array = dynamic_cast<const ArrayLiteral*>(currentNode);
			Handler_ArrayLiteral(array, signal, useEnv.get());
			break;
		}
		case NodeType::HashLiteral:
		{
			const auto* hash = dynamic_cast<const HashLiteral*>(currentNode);
			Handler_HashLiteral(hash, signal, useEnv.get());
			break;
		}
		case NodeType::IndexExpression:
		{
			const auto* index = dynamic_cast<const IndexExpression*>(currentNode);
			Handler_IndexExpression(index, signal, useEnv.get());
			break;
		}
		case NodeType::PrefixExpression:
		{
			const auto* prefix = dynamic_cast<const PrefixExpression*>(currentNode);
			Handler_PrefixExpression(prefix, signal, useEnv.get());
			break;
		}
		case NodeType::BlockStatement:
		{
			const auto* block = dynamic_cast<const BlockStatement*>(currentNode);
			Handler_BlockStatement(block, signal, useEnv.get());
			break;
		}
		case NodeType::IfExpression:
		{
			const auto* ifex = dynamic_cast<const IfExpression*>(currentNode);
			Handler_IfExpression(ifex, signal, useEnv.get());
			break;
		}
		case NodeType::InfixExpression:
		{
			const auto* infix = dynamic_cast<const InfixExpression*>(currentNode);
			Handler_InfixExpression(infix, signal, useEnv.get());
			break;
		}
		case NodeType::ReturnStatement:
		{
			const auto* ret = dynamic_cast<const ReturnStatement*>(currentNode);
			Handler_ReturnStatement(ret, signal, useEnv.get());
			break;
		}
		case NodeType::LetStatement:
		{
			const auto* let = dynamic_cast<const LetStatement*>(currentNode);
			Handler_LetStatement(let, signal, useEnv.get());
			break;
		}
		case NodeType::Identifier:
		{
			const auto* ident = dynamic_cast<const Identifier*>(currentNode);
			Handler_Identifier(ident, signal, useEnv.get());
			break;
		}
		case NodeType::FunctionLiteral:
		{
			const auto* func = dynamic_cast<const FunctionLiteral*>(currentNode);
			Handler_FunctionLiteral(func, signal, useEnv.get());
			break;
		}
		case NodeType::CallExpression:
		{
			const auto* call = dynamic_cast<const CallExpression*>(currentNode);
			Handler_CallExpression(call, signal, useEnv.get());
			break;
		}
		case NodeType::WhileExpression:
		{
			const auto* whileEx = dynamic_cast<const WhileExpression*>(currentNode);
			Handler_WhileExpression(whileEx, signal, useEnv.get());
			break;
		}
		case NodeType::ForExpression:
		{
			const auto* forEx = dynamic_cast<const ForExpression*>(currentNode);
			Handler_ForStatement(forEx, signal, useEnv.get());
			break;
		}
		case NodeType::BreakStatement:
		{
			Handler_BreakStatement(dynamic_cast<const BreakStatement*>(currentNode), signal, useEnv.get());
			break;
		}
		case NodeType::ContinueStatement:
		{
			Handler_ContinueStatement(dynamic_cast<const ContinueStatement*>(currentNode), signal, useEnv.get());
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


void StackEvaluator::Push_Eval(const INode* node, int32_t signal, const Environment* env)
{
	_stack.push({ node, signal, env });
}
void StackEvaluator::Pop_Eval()
{
	_stack.pop();
}
void StackEvaluator::Push_Result(std::shared_ptr<IObject> result)
{
	_results.push(result);
}
std::shared_ptr<IObject> StackEvaluator::Pop_Result()
{
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

void StackEvaluator::Handler_Program(const Program* program, int32_t signal, const Environment* env)
{
	for (const auto& iter : program->Statements | std::views::reverse)
	{
		Push_Eval(iter.get(), 0, env);
	}
}
void StackEvaluator::Handler_BlockStatement(const BlockStatement* block, int32_t signal, const Environment* env)
{
	if (ResultIsError() || ResultIsReturn())
	{
		return;
	}

	if (signal < block->Statements.size())
	{
		//get the statement for the current signal
		const auto& stmt = block->Statements[signal];
		Push_Eval(block, signal + 1, env);
		Push_Eval(stmt.get(), 0, env);
	}
}
void StackEvaluator::Handler_ExpressionStatement(const ExpressionStatement* expression, int32_t signal, const Environment* env)
{
	Push_Eval(expression->Expression.get(), 0, env);
}
void StackEvaluator::Handler_ReturnStatement(const ReturnStatement* ret, int32_t signal, const Environment* env)
{
	if (signal == 0)
	{
		Push_Eval(ret, 1, env);
		Push_Eval(ret->ReturnValue.get(), 0, env);
	}
	else
	{
		auto result = Pop_Result();
		Push_Result(ReturnObj::New(result));
	}
}
void StackEvaluator::Handler_LetStatement(const LetStatement* let, int32_t signal, const Environment* env)
{
	if (let->Name->NType() == NodeType::Identifier)
	{
		if (signal == 0)
		{
			Push_Eval(let, 1, env);
			Push_Eval(let->Value.get(), 0, env);
			Push_Eval(let->Name.get(), 0, env);
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
			//results.push(result);
		}
	}
	else if (let->Name->NType() == NodeType::IndexExpression)
	{
		if (signal == 0)
		{
			auto* index = dynamic_no_copy_cast<IndexExpression>(let->Name);

			Push_Eval(let, 1, env);
			Push_Eval( let->Value.get(), 0, env);
			Push_Eval( index->Left.get(), 0, env);
			Push_Eval( index->Index.get(), 0, env);
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
void StackEvaluator::Handler_Identifier(const Identifier* ident, int32_t signal, const Environment* env)
{
	if (builtIn->IsBuiltIn(ident->Value))
	{
		Push_Result(BuiltInObj::New(ident->Value));
	}
	else
	{
		auto value = env->Get(ident->Value);
		if (value != nullptr)
		{
			Push_Result(IdentifierObj::New(ident->Value, value, env));
		}
		else
		{
			Push_Result(IdentifierObj::New(ident->Value, NullObj::NULL_OBJ_REF, env));
		}
		//result = MakeError(std::format("identifier not found: {}", ident->Value), ident->BaseToken);
	}
}
void StackEvaluator::Handler_IntegerLiteral(const IntegerLiteral* integer, int32_t signal, const Environment* env)
{
	Push_Result(IntegerObj::New(integer->Value));
}
void StackEvaluator::Handler_BooleanLiteral(const BooleanLiteral* boolean, int32_t signal, const Environment* env)
{
	Push_Result(boolean->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF);
}
void StackEvaluator::Handler_StringLiteral(const StringLiteral* string, int32_t signal, const Environment* env)
{
	Push_Result(StringObj::New(string->Value));
}
void StackEvaluator::Handler_DecimalLiteral(const DecimalLiteral* decimal, int32_t signal, const Environment* env)
{
	Push_Result(DecimalObj::New(decimal->Value));
}
void StackEvaluator::Handler_PrefixExpression(const PrefixExpression* prefix, int32_t signal, const Environment* env)
{
	if (signal == 0)
	{
		Push_Eval(prefix, 1, env);
		Push_Eval(prefix->Right.get(), 0, env);
	}
	else
	{
		auto lastResult = Pop_ResultAndUnwrap();
		Push_Result(EvalPrefixExpression(prefix->BaseToken, lastResult));
	}
}
void StackEvaluator::Handler_InfixExpression(const InfixExpression* infix, int32_t signal, const Environment* env)
{
	if (signal == 0)
	{
		Push_Eval(infix, 1, env);
		Push_Eval(infix->Left.get(), 0, env);
	}
	else if (signal == 1)
	{
		Push_Eval(infix, 2, env);
		Push_Eval(infix->Right.get(), 0, env);
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
void StackEvaluator::Handler_IfExpression(const IfExpression* ifExpr, int32_t signal, const Environment* env)
{
	if (signal == 0)
	{
		Push_Eval(ifExpr,  1, env);
		Push_Eval(ifExpr->Condition.get(), 0, env);
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
			Push_Eval(ifExpr->Consequence.get(), 0, env);
		}
		else if (ifExpr->Alternative != nullptr)
		{
			Push_Eval(ifExpr->Alternative.get(), 0, env);
		}
		else
		{
			Push_Result(NullObj::NULL_OBJ_REF);
		}
	}
}
void StackEvaluator::Handler_FunctionLiteral(const FunctionLiteral* function, int32_t signal, const Environment* env)
{

	Push_Result(FunctionObj::New(function->Parameters, function->Body, env));

}
void StackEvaluator::Handler_CallExpression(const CallExpression* call, int32_t signal, const Environment* env)
{
	if (signal == 0)
	{
		Push_Eval(call, 1, env);
		Push_Eval(call->Function.get(), 0, env);
	}
	else if (signal == 1)
	{
		auto lastResult = Pop_Result();

		lastResult = UnwrapIfIdentObj(lastResult);

		if (lastResult->Type() != ObjectType::FUNCTION_OBJ && lastResult->Type() != ObjectType::BUILTIN_OBJ)
		{
			Push_Result(MakeError(std::format("literal not a function: {}", lastResult->Inspect()), call->BaseToken));
			return;
		}
		auto function = lastResult;

		Push_Eval(call, 2, env);
		// Evaluate arguments
		for (const auto& iter : call->Arguments)
		{
			Push_Eval(iter.get(), 0, env);
		}
	}
	else
	{
		std::vector<std::shared_ptr<IObject>> evalArgs;
		//if (results.size() < call->Arguments.size())
		//{
		//	results.push(MakeError("not enough arguments", call->BaseToken));
		//	break;
		//}

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
			auto builtInToCall = builtIn->GetBuiltInFunction(func->Name);
			if (builtInToCall == nullptr)
			{
				Push_Result(MakeError(std::format("unknown function: {}", func->Name), call->BaseToken));
				return;
			}
			auto result = builtInToCall(evalArgs, call->BaseToken);
			if (result != VoidObj::VOID_OBJ_REF) //check for a "void" return
			{
				Push_Result(result);
			}
		}
		else
		{
			auto func = std::dynamic_pointer_cast<FunctionObj>(function);
			auto extEnv = ExtendFunctionEnv(func, evalArgs);
			Push_Eval(func->Body.get(), 0, extEnv.get());
		}
	}

}
void StackEvaluator::Handler_ArrayLiteral(const ArrayLiteral* array, int32_t signal, const Environment* env)
{
	std::vector<std::shared_ptr<IObject>> elements;

	if (signal == 0)
	{
		Push_Eval(array, 1, env);

		for (const auto& elem : array->Elements)
		{
			Push_Eval(elem.get(), 0, env);
		}
	}
	else
	{
		std::vector<std::shared_ptr<IObject>> evalArgs;
		if (results.size() < array->Elements.size())
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
		results.push(ArrayObj::New(evalArgs));
	}
}
void StackEvaluator::Handler_IndexExpression(const IndexExpression* index, int32_t signal, const Environment* env)
{
	if (signal == 0)
	{
		Push_Eval(index, 1, env);
		Push_Eval(index->Left.get(), 0, env);
		Push_Eval(index->Index.get(), 0, env);

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
void StackEvaluator::Handler_HashLiteral(const HashLiteral* hash, int32_t signal, const Environment* env)
{
	std::vector<std::shared_ptr<IObject>> elements;

	if (signal == 0)
	{
		Push_Eval(hash, 1, env);
		for (const auto& [key, value] : hash->Elements)
		{
			Push_Eval(key.get(), 0, env);
			Push_Eval(value.get(), 0, env);
		}
	}
	else
	{
		std::unordered_map<HashKey, HashEntry> evalArgs;
		if (results.size() < hash->Elements.size())
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
void StackEvaluator::Handler_NullLiteral(const NullLiteral* null, int32_t signal, const Environment* env)
{
	Push_Result(NullObj::NULL_OBJ_REF);
}
void StackEvaluator::Handler_WhileExpression(const WhileExpression* whileExp, int32_t signal, const Environment* env)
{
	if (signal == 0)
	{
		//check the action for return/error
		if (ResultIsReturn())
		{
			auto top = Pop_Result();

			auto* ret = dynamic_no_copy_cast<ReturnObj>(top);
			if (!(ret->Value == ContinueObj::CONTINUE_OBJ_REF || ret->Value == BreakObj::BREAK_OBJ_REF))
			{
				Push_Result(top);
			}
		}

		if (ResultIsError())
		{
			return;
		}

		Push_Eval(whileExp, 1, env);
		Push_Eval(whileExp->Condition.get(), 0, env);
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
			Push_Eval(whileExp, 0, env);
			Push_Eval(whileExp->Action.get(), 0, env);
		}
	}
}
void StackEvaluator::Handler_ForStatement(const ForExpression* forExp, int32_t signal, const Environment* env)
{
	if (signal == 0)
	{
		Push_Eval(forExp, 1, env);
		Push_Eval(forExp->Init.get(), 0, env);
	}
	else if (signal == 1)
	{
		//condition
		Push_Eval(forExp, 2, env);
		Push_Eval(forExp->Condition.get(), 0, env);
	}
	else if (signal == 2)
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
			Push_Eval(forExp, 3, env);
			Push_Eval(forExp->Action.get(), 0, env);
		}
	}
	else if (signal == 3)
	{
		//check the action for return/error
		if (ResultIsReturn())
		{
			auto top = Pop_Result();

			auto* ret = dynamic_no_copy_cast<ReturnObj>(top);
			if (!(ret->Value == ContinueObj::CONTINUE_OBJ_REF || ret->Value == BreakObj::BREAK_OBJ_REF))
			{
				Push_Result(top);
			}
		}

		if (ResultIsError())
		{
			return;
		}
		Push_Eval(forExp, 1, env);
		Push_Eval(forExp->Post.get(), 0, env);
	}
}
void StackEvaluator::Handler_ContinueStatement(const ContinueStatement* cont, int32_t signal, const Environment* env)
{
	Push_Result(ReturnObj::New(ContinueObj::CONTINUE_OBJ_REF));
}
void StackEvaluator::Handler_BreakStatement(const BreakStatement* brk, int32_t signal, const Environment* env)
{
	Push_Result(ReturnObj::New(BreakObj::BREAK_OBJ_REF));
}