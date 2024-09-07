#include "Evaluator.h"
#include <stack>

std::shared_ptr<IObject> Evaluator::Eval(Program& program, std::shared_ptr<Environment>& env)
{
	std::shared_ptr<IObject> result = nullptr;
	for (const auto& stmt : program.Statements)
	{
		result = StackEval(stmt, env);
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

std::shared_ptr<IObject> Evaluator::StackEval(const std::shared_ptr<INode>& node, std::shared_ptr<Environment>& env)
{
	std::stack<std::tuple<std::shared_ptr<INode>,int32_t>> stack;
	std::stack<std::shared_ptr<IObject>> results;

	stack.push({ node, 0 });

	while (!stack.empty())
	{
		auto [currentNode, signal] = stack.top();
		stack.pop();

		if (!results.empty() && results.top()->Type() == ObjectType::ERROR_OBJ)
		{
			return results.top();
		}

		auto type = currentNode->NType();
		switch (type)
		{
			case NodeType::Program:
			{
				auto program = std::dynamic_pointer_cast<Program>(currentNode);
				for (auto it = program->Statements.rbegin(); it != program->Statements.rend(); ++it)
				{
					stack.push({ *it, 0 });
				}
				break;
			}
			case NodeType::ExpressionStatement:
			{
				auto expression = std::dynamic_pointer_cast<ExpressionStatement>(currentNode);
				stack.push({ expression->Expression, 0 });
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
					stack.push({ prefix, 1 });
					stack.push({ prefix->Right, 0 });
				}
				else
				{
					auto lastResult = results.top();
					results.pop();
					results.push(EvalPrefixExpression(prefix->Token, lastResult));
				}
				break;
			}
			case NodeType::BlockStatement:
			{
				auto block = std::dynamic_pointer_cast<BlockStatement>(currentNode);
				for (auto it = block->Statements.begin(); it != block->Statements.end(); ++it)
				{
					stack.push({ *it, 0 });
				}
				break;
			}
			case NodeType::IfExpression:
			{
				auto ifex = std::dynamic_pointer_cast<IfExpression>(currentNode);
				if (signal == 0)
				{
					stack.push({ ifex,  1 });
					stack.push({ ifex->Condition, 0 });
				}
				else
				{
					auto lastResult = results.top();
					results.pop();
					if (IsTruthy(lastResult))
					{
						stack.push({ ifex->Consequence, 0 });
					}
					else if (ifex->Alternative != nullptr)
					{
						stack.push({ ifex->Alternative, 0 });
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
					stack.push({ infix, 1 });
					stack.push({ infix->Left, 0 });
				}
				else if (signal == 1)
				{
					stack.push({ infix, 2 });
					stack.push({ infix->Right, 0 });
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

					auto left = results.top();
					results.pop();

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
					stack.push({ ret, 1 });
					stack.push({ ret->ReturnValue, 0 });
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
					stack.push({ let, 1 });
					stack.push({ let->Value, 0 });
				}
				else
				{
					auto lastResult = results.top();
					results.pop();
					env->Set(let->Name->Value, lastResult);
				}
				break;
			}
			case NodeType::Identifier:
			{
				auto ident = std::dynamic_pointer_cast<Identifier>(currentNode);
				auto value = env->Get(ident->Value);
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
				results.push(FunctionObj::New(func->Parameters, func->Body, env));
				break;
			}
			case NodeType::CallExpression:
			{
				auto call = std::dynamic_pointer_cast<CallExpression>(currentNode);
				if (signal == 0)
				{
					stack.push({ call, 1 });
					stack.push({ call->Function, 0 });
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

					stack.push({ call, 2 });

					// Evaluate arguments
					for (auto it = call->Arguments.rbegin(); it != call->Arguments.rend(); ++it)
					{
						stack.push({ *it, 0 });
					}
				}
				else
				{
					std::vector<std::shared_ptr<IObject>> evalArgs;
					for (size_t i = 0; i < call->Arguments.size(); i++)
					{
						auto arg = results.top();
						results.pop();
						evalArgs.push_back(arg);
					}
					auto function = results.top();
					results.pop();

					auto fn = std::dynamic_pointer_cast<FunctionObj>(function);
					auto result = ApplyFunction(fn, evalArgs);
					results.push(result);
				}
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

	//if (results.size() > 1)
	//{
	//	return MakeError("stack has more than one result", Token());
	//}

	auto result = results.top();
	return result;
}

std::shared_ptr<IObject> Evaluator::Eval(const std::shared_ptr<INode>& node, std::shared_ptr<Environment>& env)
{
	auto type = node->NType();
	std::shared_ptr<IObject> result = nullptr;
	switch (type)
	{
		case NodeType::Program:
		{
			auto program = std::dynamic_pointer_cast<Program>(node);
			result = Eval(*program, env);
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
			if (condition->Type() == ObjectType::ERROR_OBJ)
			{
				return condition;
			}

			if (IsTruthy(condition))
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
			if (left->Type() == ObjectType::ERROR_OBJ)
			{
				return left;
			}
			auto right = Eval(infix->Right, env);
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
		default:
		{
			result = NullObj::NULL_OBJ_REF;
		}
	}
		return result;
}

std::shared_ptr<IObject> Evaluator::EvalPrefixExpression(const Token& op, const std::shared_ptr<IObject>& right)
{
	if (op.Type == TokenType::TOKEN_BANG)
	{
		return EvalBangOperatorExpression(op, right);
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

std::shared_ptr<IObject> Evaluator::EvalInfixExpression(const Token& op, const std::shared_ptr<IObject>& left, const std::shared_ptr<IObject>& right)
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

bool Evaluator::IsTruthy(const std::shared_ptr<IObject>& obj) const
{
	auto booleanObj = EvalAsBoolean(obj);
	if (booleanObj == BooleanObj::TRUE_OBJ_REF)
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::shared_ptr<BooleanObj> Evaluator::EvalAsBoolean(const std::shared_ptr<IObject>& obj) const
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
}

std::shared_ptr<IObject> Evaluator::EvalIntegerInfixExpression(const Token& op, const std::shared_ptr<IntegerObj>& left, const std::shared_ptr<IntegerObj>& right)
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

std::shared_ptr<IObject> Evaluator::EvalBooleanInfixExpression(const Token& op, const std::shared_ptr<BooleanObj>& left, const std::shared_ptr<BooleanObj>& right)
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

std::shared_ptr<IObject> Evaluator::EvalBangOperatorExpression(const Token& op, const std::shared_ptr<IObject>& right)
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

std::shared_ptr<IObject> Evaluator::EvalMinusPrefixOperatorExpression(const Token& op, const std::shared_ptr<IObject>& right)
{
	if (right->Type() != ObjectType::INTEGER_OBJ)
	{
		return MakeError(std::format("unknown operator: {}{}", op.Literal, right->Type().Name), op);
	}

	auto value = std::dynamic_pointer_cast<IntegerObj>(right)->Value;
	return IntegerObj::New(-value);
}

std::shared_ptr<IObject> Evaluator::MakeError(const std::string& message, const Token& token)
{
	return ErrorObj::New(message, token);
}

std::vector<std::shared_ptr<IObject>> Evaluator::EvalExpressions(std::vector<std::shared_ptr<IExpression>>& expressions, std::shared_ptr<Environment>& env)
{
	std::vector<std::shared_ptr<IObject>> result;
	for (const auto& expr : expressions)
	{
		auto evaluated = Eval(expr, env);
		if (evaluated->Type() == ObjectType::ERROR_OBJ)
		{
			return { evaluated };
		}
		result.push_back(evaluated);
	}
	return result;
}

std::shared_ptr<IObject> Evaluator::ApplyFunction(std::shared_ptr<FunctionObj> fn, std::vector<std::shared_ptr<IObject>>& args)
{
	auto extEnv = ExtendFunctionEnv(fn, args);
	auto evaluated = Eval(fn->Body, extEnv);
	return evaluated;
}

std::shared_ptr<Environment> Evaluator::ExtendFunctionEnv(std::shared_ptr<FunctionObj> fn, std::vector<std::shared_ptr<IObject>>& args)
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
