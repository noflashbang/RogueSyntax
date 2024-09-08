#include "Evaluator.h"
#include <stack>

std::shared_ptr<IObject> Evaluator::Eval(Program& program, std::shared_ptr<Environment>& env)  const
{
	std::shared_ptr<IObject> result = nullptr;
	for (const auto& stmt : program.Statements)
	{
		result = Eval(stmt, env);
		if (result == nullptr)
		{
			continue;
		}

		if (result->Type() == ObjectType::RETURN_OBJ)
		{
			result = std::dynamic_pointer_cast<ReturnObj>(result)->Value;
			break;
		}
		if (result->Type() == ObjectType::ERROR_OBJ)
		{
			break;
		}
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::EvalPrefixExpression(const Token& op, const std::shared_ptr<IObject>& right) const
{
	if (op.Type == TokenType::TOKEN_BANG)
	{
		return EvalBangPrefixOperatorExpression(op, right);
	}
	else if (op.Type == TokenType::TOKEN_MINUS)
	{
		return EvalMinusPrefixOperatorExpression(op, right);
	}
	else
	{
		return MakeError(std::format("unknown operator: {}", op.Literal), op);
	}
}

std::shared_ptr<IObject> Evaluator::EvalInfixExpression(const Token& op, const std::shared_ptr<IObject>& left, const std::shared_ptr<IObject>& right) const
{
	if (left->Type() != right->Type())
	{
		return MakeError(std::format("type mismatch: {} {} {}", left->Type().Name, op.Literal, right->Type().Name), op);
	}

	if (left->Type() == ObjectType::INTEGER_OBJ && right->Type() == ObjectType::INTEGER_OBJ)
	{
		return EvalIntegerInfixExpression(op, std::dynamic_pointer_cast<IntegerObj>(left), std::dynamic_pointer_cast<IntegerObj>(right));
	}
	else if (left->Type() == ObjectType::BOOLEAN_OBJ && right->Type() == ObjectType::BOOLEAN_OBJ)
	{
		return EvalBooleanInfixExpression(op, std::dynamic_pointer_cast<BooleanObj>(left), std::dynamic_pointer_cast<BooleanObj>(right));
	}
	else
	{
		return MakeError(std::format("unknown operator: {} {} {}", left->Type().Name, op.Literal, right->Type().Name), op);
	}
}

std::shared_ptr<IObject> Evaluator::EvalAsBoolean(const Token& context, const std::shared_ptr<IObject>& obj) const
{
	if (obj == NullObj::NULL_OBJ_REF)
	{
		return BooleanObj::FALSE_OBJ_REF;
	}

	if (obj == BooleanObj::TRUE_OBJ_REF)
	{
		return BooleanObj::TRUE_OBJ_REF;
	}
	
	if(obj == BooleanObj::FALSE_OBJ_REF)
	{
		return BooleanObj::FALSE_OBJ_REF;
	}

	if (obj->Type() == ObjectType::INTEGER_OBJ)
	{
		auto value = std::dynamic_pointer_cast<IntegerObj>(obj)->Value;
		return value == 0 ? BooleanObj::FALSE_OBJ_REF : BooleanObj::TRUE_OBJ_REF;
	}

	return MakeError(std::format("illegal expression, type {} can not be evaluated as boolean: {}", obj->Type().Name, obj->Inspect()), context);
}

std::shared_ptr<IObject> Evaluator::EvalIntegerInfixExpression(const Token& op, const std::shared_ptr<IntegerObj>& left, const std::shared_ptr<IntegerObj>& right) const
{
	if (op.Type == TokenType::TOKEN_PLUS)
	{
		return IntegerObj::New(left->Value + right->Value);
	}
	else if (op.Type == TokenType::TOKEN_MINUS)
	{
		return IntegerObj::New(left->Value - right->Value);
	}
	else if (op.Type == TokenType::TOKEN_ASTERISK)
	{
		return IntegerObj::New(left->Value * right->Value);
	}
	else if (op.Type == TokenType::TOKEN_SLASH)
	{
		return IntegerObj::New(left->Value / right->Value);
	}
	else if (op.Type == TokenType::TOKEN_LT)
	{
		return left->Value < right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (op.Type == TokenType::TOKEN_GT)
	{
		return left->Value > right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (op.Type == TokenType::TOKEN_EQ)
	{
		return left->Value == right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (op.Type == TokenType::TOKEN_NOT_EQ)
	{
		return left->Value != right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else
	{
		return MakeError(std::format("unknown operator: {} {} {}", left->Type().Name, op.Literal, right->Type().Name), op);
	}
}

std::shared_ptr<IObject> Evaluator::EvalBooleanInfixExpression(const Token& op, const std::shared_ptr<BooleanObj>& left, const std::shared_ptr<BooleanObj>& right) const
{
	if (op.Type == TokenType::TOKEN_EQ)
	{
		return left->Value == right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else if (op.Type == TokenType::TOKEN_NOT_EQ)
	{
		return left->Value != right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
	}
	else
	{
		return MakeError(std::format("unknown operator: {} {} {}", left->Type().Name, op.Literal, right->Type().Name), op);
	}
}

std::shared_ptr<IObject> Evaluator::EvalBangPrefixOperatorExpression(const Token& op, const std::shared_ptr<IObject>& right) const
{
	if (right == BooleanObj::TRUE_OBJ_REF)
	{
		return BooleanObj::FALSE_OBJ_REF;
	}
	else if (right == BooleanObj::FALSE_OBJ_REF)
	{
		return BooleanObj::TRUE_OBJ_REF;
	}
	else if (right == NullObj::NULL_OBJ_REF)
	{
		return BooleanObj::TRUE_OBJ_REF;
	}
	else if (right->Type() == ObjectType::INTEGER_OBJ)
	{
		auto value = std::dynamic_pointer_cast<IntegerObj>(right)->Value;
		if (value == 0)
		{
			return BooleanObj::TRUE_OBJ_REF;
		}
		else
		{
			return BooleanObj::FALSE_OBJ_REF;
		}
	}
	else
	{
		return MakeError(std::format("unknown operator: {}{}", op.Literal, right->Type().Name), op);
	}
}

std::shared_ptr<IObject> Evaluator::EvalMinusPrefixOperatorExpression(const Token& op, const std::shared_ptr<IObject>& right) const
{
	if (right->Type() != ObjectType::INTEGER_OBJ)
	{
		return MakeError(std::format("unknown operator: {}{}", op.Literal, right->Type().Name), op);
	}

	auto value = std::dynamic_pointer_cast<IntegerObj>(right)->Value;
	return IntegerObj::New(-value);
}

std::shared_ptr<IObject> Evaluator::MakeError(const std::string& message, const Token& token) const
{
	return ErrorObj::New(message, token);
}

std::shared_ptr<Environment> Evaluator::ExtendFunctionEnv(std::shared_ptr<FunctionObj> fn, std::vector<std::shared_ptr<IObject>>& args) const
{
	auto env = Environment::NewEnclosed(fn->Env);
	for (size_t i = 0; i < fn->Parameters.size(); i++)
	{
		auto param = fn->Parameters[i].get();

		if (param->NType() != NodeType::Identifier)
		{
			continue;
		}

		auto ident = dynamic_cast<Identifier*>(param);
		env->Set(ident->Value, args[i]);
	}
	return env;
}

std::shared_ptr<IObject> Evaluator::UnwrapIfReturnObj(const std::shared_ptr<IObject>& input) const
{
	if (input != nullptr && input->Type() == ObjectType::RETURN_OBJ)
	{
		return std::dynamic_pointer_cast<ReturnObj>(input)->Value;
	}
	return input;
}

std::shared_ptr<IObject> StackEvaluator::Eval(const std::shared_ptr<INode>& node, std::shared_ptr<Environment>& env) const
{
	std::stack<std::tuple<std::shared_ptr<INode>, int32_t, std::shared_ptr<Environment>>> stack;
	std::stack<std::shared_ptr<IObject>> results;

	stack.push({ node, 0, nullptr });

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
			auto program = std::dynamic_pointer_cast<Program>(currentNode);
			for (auto it = program->Statements.rbegin(); it != program->Statements.rend(); ++it)
			{
				stack.push({ *it, 0, useEnv });
			}
			break;
		}
		case NodeType::ExpressionStatement:
		{
			auto expression = std::dynamic_pointer_cast<ExpressionStatement>(currentNode);
			stack.push({ expression->Expression, 0, useEnv });
			break;
		}
		case NodeType::IntegerLiteral:
		{
			auto integer = std::dynamic_pointer_cast<IntegerLiteral>(currentNode);
			results.push(IntegerObj::New(integer->Value));
			break;
		}
		case NodeType::BooleanLiteral:
		{
			auto boolean = std::dynamic_pointer_cast<BooleanLiteral>(currentNode);
			results.push(boolean->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF);
			break;
		}
		case NodeType::PrefixExpression:
		{
			auto prefix = std::dynamic_pointer_cast<PrefixExpression>(currentNode);
			if (signal == 0)
			{
				stack.push({ prefix, 1, useEnv });
				stack.push({ prefix->Right, 0, useEnv });
			}
			else
			{
				auto lastResult = results.top();
				results.pop();

				//unwrap any return objects
				lastResult = UnwrapIfReturnObj(lastResult);

				results.push(EvalPrefixExpression(prefix->Token, lastResult));
			}
			break;
		}
		case NodeType::BlockStatement:
		{
			auto block = std::dynamic_pointer_cast<BlockStatement>(currentNode);

			//check the action for return/error
			if (!results.empty() && (results.top()->Type() == ObjectType::ERROR_OBJ || results.top()->Type() == ObjectType::RETURN_OBJ))
			{
				break;
			}

			if (signal < block->Statements.size())
			{
				//get the statement for the current signal
				auto stmt = block->Statements[signal];

				stack.push({ block, signal + 1, useEnv });
				stack.push({ stmt, 0, useEnv });
			}
			break;
		}
		case NodeType::IfExpression:
		{
			auto ifex = std::dynamic_pointer_cast<IfExpression>(currentNode);
			if (signal == 0)
			{
				stack.push({ ifex,  1, useEnv });
				stack.push({ ifex->Condition, 0, useEnv });
			}
			else
			{
				auto lastResult = results.top();
				results.pop();

				//unwrap any return objects
				lastResult = UnwrapIfReturnObj(lastResult);

				auto evalBool = EvalAsBoolean(ifex->Token, lastResult);

				if (evalBool->Type() == ObjectType::ERROR_OBJ)
				{
					results.push(evalBool);
					break;
				}

				if (evalBool == BooleanObj::TRUE_OBJ_REF)
				{
					stack.push({ ifex->Consequence, 0, useEnv });
				}
				else if (ifex->Alternative != nullptr)
				{
					stack.push({ ifex->Alternative, 0, useEnv });
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
			auto infix = std::dynamic_pointer_cast<InfixExpression>(currentNode);
			if (signal == 0)
			{
				stack.push({ infix, 1, useEnv });
				stack.push({ infix->Left, 0, useEnv });
			}
			else if (signal == 1)
			{
				stack.push({ infix, 2, useEnv });
				stack.push({ infix->Right, 0, useEnv });
			}
			else
			{
				if (results.size() < 2)
				{
					results.push(MakeError("infix expression requires two operands", infix->Token));
					break;
				}

				auto right = results.top();
				results.pop();

				right = UnwrapIfReturnObj(right);

				auto left = results.top();
				results.pop();

				left = UnwrapIfReturnObj(left);

				auto result = EvalInfixExpression(infix->Token, left, right);
				results.push(result);
			}
			break;
		}
		case NodeType::ReturnStatement:
		{
			auto ret = std::dynamic_pointer_cast<ReturnStatement>(currentNode);
			if (signal == 0)
			{
				stack.push({ ret, 1, useEnv });
				stack.push({ ret->ReturnValue, 0, useEnv });
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
			auto let = std::dynamic_pointer_cast<LetStatement>(currentNode);
			if (signal == 0)
			{
				stack.push({ let, 1, useEnv });
				stack.push({ let->Value, 0, useEnv });
			}
			else
			{
				auto lastResult = results.top();
				results.pop();

				lastResult = UnwrapIfReturnObj(lastResult);

				useEnv->Set(let->Name->Value, lastResult);
			}
			break;
		}
		case NodeType::Identifier:
		{
			auto ident = std::dynamic_pointer_cast<Identifier>(currentNode);
			auto value = useEnv->Get(ident->Value);
			if (value != nullptr)
			{
				results.push(value);
			}
			else
			{
				results.push(MakeError(std::format("identifier not found: {}", ident->Value), ident->Token));
			}
			break;
		}
		case NodeType::FunctionLiteral:
		{
			auto func = std::dynamic_pointer_cast<FunctionLiteral>(currentNode);
			results.push(FunctionObj::New(func->Parameters, func->Body, useEnv));
			break;
		}
		case NodeType::CallExpression:
		{
			auto call = std::dynamic_pointer_cast<CallExpression>(currentNode);
			if (signal == 0)
			{
				stack.push({ call, 1, useEnv });
				stack.push({ call->Function, 0, useEnv });
			}
			else if (signal == 1)
			{
				auto lastResult = results.top();

				if (lastResult->Type() != ObjectType::FUNCTION_OBJ)
				{
					results.push(MakeError(std::format("literal not a function: {}", lastResult->Inspect()), call->Token));
					break;
				}
				auto function = lastResult;

				stack.push({ call, 2, useEnv });

				// Evaluate arguments
				for (auto it = call->Arguments.rbegin(); it != call->Arguments.rend(); ++it)
				{
					stack.push({ *it, 0, useEnv });
				}
			}
			else
			{
				std::vector<std::shared_ptr<IObject>> evalArgs;

				if (results.size() < call->Arguments.size())
				{
					results.push(MakeError("not enough arguments", call->Token));
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

					evalArgs.push_back(arg);
				}
				auto function = results.top();
				results.pop();

				auto fn = std::dynamic_pointer_cast<FunctionObj>(function);

				auto extEnv = ExtendFunctionEnv(fn, evalArgs);
				stack.push({ fn->Body, 0, extEnv });
			}
			break;
		}
		case NodeType::WhileExpression:
		{
			auto whileEx = std::dynamic_pointer_cast<WhileExpression>(currentNode);
			if (signal == 0)
			{
				//check the action for return/error
				if (!results.empty() && (results.top()->Type() == ObjectType::ERROR_OBJ || results.top()->Type() == ObjectType::RETURN_OBJ))
				{
					if (results.top()->Type() == ObjectType::RETURN_OBJ)
					{
						auto ret = std::dynamic_pointer_cast<ReturnObj>(results.top());
						if (ret->Value == BreakObj::BREAK_OBJ_REF)
						{
							results.pop();
							break;
						}
						else if (ret->Value == ContinueObj::CONTINUE_OBJ_REF)
						{
							results.pop();
						}
						else
						{
							break;
						}
					}
					else if (results.top()->Type() == ObjectType::ERROR_OBJ)
					{
						break;
					}
				}
				stack.push({ whileEx, 1, useEnv });
				stack.push({ whileEx->Condition, 0, useEnv });
			}
			else
			{
				auto lastResult = results.top();
				results.pop();

				lastResult = UnwrapIfReturnObj(lastResult);

				auto booleanObj = EvalAsBoolean(whileEx->Token, lastResult);

				if (booleanObj->Type() == ObjectType::ERROR_OBJ)
				{
					results.push(booleanObj);
					break;
				}

				if (booleanObj == BooleanObj::TRUE_OBJ_REF)
				{
					stack.push({ whileEx, 0, useEnv });
					stack.push({ whileEx->Action, 0, useEnv });
				}
			}
			break;
		}
		case NodeType::ForExpression:
		{
			auto forEx = std::dynamic_pointer_cast<ForExpression>(currentNode);
			if (signal == 0)
			{
				stack.push({ forEx, 1, useEnv });
				stack.push({ forEx->Init, 0, useEnv });
			}
			else if (signal == 1)
			{
				//condition
				stack.push({ forEx, 2, useEnv });
				stack.push({ forEx->Condition, 0, useEnv });
			}
			else if (signal == 2)
			{
				auto lastResult = results.top();
				results.pop();

				lastResult = UnwrapIfReturnObj(lastResult);

				auto booleanObj = EvalAsBoolean(forEx->Token, lastResult);

				if (booleanObj->Type() == ObjectType::ERROR_OBJ)
				{
					results.push(booleanObj);
					break;
				}

				if (booleanObj == BooleanObj::TRUE_OBJ_REF)
				{
					stack.push({ forEx, 3, useEnv });
					stack.push({ forEx->Action, 0, useEnv });
				}
			}
			else if (signal == 3)
			{
				//check the action for return/error
				if (!results.empty() && (results.top()->Type() == ObjectType::ERROR_OBJ || results.top()->Type() == ObjectType::RETURN_OBJ))
				{
					if (results.top()->Type() == ObjectType::RETURN_OBJ)
					{
						auto ret = std::dynamic_pointer_cast<ReturnObj>(results.top());
						if (ret->Value == BreakObj::BREAK_OBJ_REF)
						{
							results.pop();
							break;
						}
						else if (ret->Value == ContinueObj::CONTINUE_OBJ_REF)
						{
							results.pop();
						}
						else
						{
							break;
						}
					}
					else if (results.top()->Type() == ObjectType::ERROR_OBJ)
					{
						break;
					}
				}
				stack.push({ forEx, 1, useEnv });
				stack.push({ forEx->Post, 0, useEnv });
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

	auto result = results.top();
	return result;
}


std::shared_ptr<IObject> RecursiveEvaluator::Eval(const std::shared_ptr<INode>& node, std::shared_ptr<Environment>& env) const
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
			auto expression = std::dynamic_pointer_cast<ExpressionStatement>(node);
			result = Eval(expression->Expression, env);
			break;
		}
		case NodeType::IntegerLiteral:
		{
			auto integer = std::dynamic_pointer_cast<IntegerLiteral>(node);
			result = IntegerObj::New(integer->Value);
			break;
		}
		case NodeType::BooleanLiteral:
		{
			auto boolean = std::dynamic_pointer_cast<BooleanLiteral>(node);
			result = boolean->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
			break;
		}
		case NodeType::PrefixExpression:
		{
			auto prefix = std::dynamic_pointer_cast<PrefixExpression>(node);
			auto right = Eval(prefix->Right, env);

			right = UnwrapIfReturnObj(right);

			if (right->Type() == ObjectType::ERROR_OBJ)
			{
				return right;
			}
			result = EvalPrefixExpression(prefix->Token, right);
			break;
		}
		case NodeType::BlockStatement:
		{
			auto block = std::dynamic_pointer_cast<BlockStatement>(node);
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
			auto ifex = std::dynamic_pointer_cast<IfExpression>(node);

			auto condition = Eval(ifex->Condition, env);

			condition = UnwrapIfReturnObj(condition);

			if (condition->Type() == ObjectType::ERROR_OBJ)
			{
				return condition;
			}

			auto booleanObj = EvalAsBoolean(ifex->Token, condition);

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
			auto infix = std::dynamic_pointer_cast<InfixExpression>(node);
			auto left = Eval(infix->Left, env);

			left = UnwrapIfReturnObj(left);

			if (left->Type() == ObjectType::ERROR_OBJ)
			{
				return left;
			}
			auto right = Eval(infix->Right, env);

			right = UnwrapIfReturnObj(right);

			if (right->Type() == ObjectType::ERROR_OBJ)
			{
				return right;
			}
			result = EvalInfixExpression(infix->Token, left, right);
			break;
		}
		case NodeType::ReturnStatement:
		{
			auto ret = std::dynamic_pointer_cast<ReturnStatement>(node);
			auto value = Eval(ret->ReturnValue, env);
			result = ReturnObj::New(value);
			break;
		}
		case NodeType::LetStatement:
		{
			auto let = std::dynamic_pointer_cast<LetStatement>(node);
			auto value = Eval(let->Value, env);

			value = UnwrapIfReturnObj(value);

			if (value->Type() == ObjectType::ERROR_OBJ)
			{
				return value;
			}
			env->Set(let->Name->Value, value);
			break;
		}
		case NodeType::Identifier:
		{
			auto ident = std::dynamic_pointer_cast<Identifier>(node);
			auto value = env->Get(ident->Value);
			if (value != nullptr)
			{
				result = value;
			}
			else
			{
				result = MakeError(std::format("identifier not found: {}", ident->Value), ident->Token);
			}
			break;
		}
		case NodeType::FunctionLiteral:
		{
			auto func = std::dynamic_pointer_cast<FunctionLiteral>(node);
			result = FunctionObj::New(func->Parameters, func->Body, env);
			break;
		}
		case NodeType::CallExpression:
		{
			auto call = std::dynamic_pointer_cast<CallExpression>(node);
			auto function = Eval(call->Function, env);
			if (function->Type() == ObjectType::ERROR_OBJ)
			{
				return function;
			}

			if (function->Type() != ObjectType::FUNCTION_OBJ)
			{
				return MakeError(std::format("literal not a function: {}", function->Inspect()), call->Token);
			}

			auto evalArgs = EvalExpressions(call->Arguments, env);
			if (evalArgs.size() == 1 && evalArgs[0]->Type() == ObjectType::ERROR_OBJ)
			{
				return evalArgs[0];
			}

			auto fn = std::dynamic_pointer_cast<FunctionObj>(function);
			result = ApplyFunction(fn, evalArgs);
			break;
		}
		case NodeType::WhileExpression:
		{
			auto whileEx = std::dynamic_pointer_cast<WhileExpression>(node);
			auto condition = Eval(whileEx->Condition, env);

			condition = UnwrapIfReturnObj(condition);

			if (condition->Type() == ObjectType::ERROR_OBJ)
			{
				return condition;
			}

			auto booleanObj = EvalAsBoolean(whileEx->Token, condition);

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
						auto ret = std::dynamic_pointer_cast<ReturnObj>(evaluated);
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

				if (condition->Type() == ObjectType::ERROR_OBJ)
				{
					return condition;
				}

				booleanObj = EvalAsBoolean(whileEx->Token, condition);

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
			auto forEx = std::dynamic_pointer_cast<ForExpression>(node);

			auto init = Eval(forEx->Init, env);

			auto condition = Eval(forEx->Condition, env);

			condition = UnwrapIfReturnObj(condition);

			if (condition->Type() == ObjectType::ERROR_OBJ)
			{
				return condition;
			}

			auto booleanObj = EvalAsBoolean(forEx->Token, condition);

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
						auto ret = std::dynamic_pointer_cast<ReturnObj>(evaluated);
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

				if (condition->Type() == ObjectType::ERROR_OBJ)
				{
					return condition;
				}

				booleanObj = EvalAsBoolean(forEx->Token, condition);

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
	return result;
}

std::vector<std::shared_ptr<IObject>> RecursiveEvaluator::EvalExpressions(std::vector<std::shared_ptr<IExpression>>& expressions, std::shared_ptr<Environment>& env) const
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
			result.push_back(ret->Value);
		}
		else
		{
			result.push_back(evaluated);
		}
	}
	return result;
}

std::shared_ptr<IObject> RecursiveEvaluator::ApplyFunction(std::shared_ptr<FunctionObj> fn, std::vector<std::shared_ptr<IObject>>& args) const
{
	auto extEnv = ExtendFunctionEnv(fn, args);
	auto evaluated = Eval(fn->Body, extEnv);
	return evaluated;
}