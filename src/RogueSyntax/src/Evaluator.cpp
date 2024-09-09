#include "Evaluator.h"
#include "pch.h"

std::shared_ptr<IObject> Evaluator::Eval(const std::shared_ptr<Program>& program, const std::shared_ptr<Environment>& env)  const
{
	std::shared_ptr<IObject> result = nullptr;
	for (const auto& stmt : program->Statements)
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
	if (left->Type() != right->Type())
	{
		result = MakeError(std::format("type mismatch: {} {} {}", left->Type().Name, optor.Literal, right->Type().Name), optor);
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

std::shared_ptr<IObject> Evaluator::EvalAsBoolean(const Token& context, const std::shared_ptr<IObject>& obj) const
{
	std::shared_ptr<IObject> result = nullptr;
	if (obj == NullObj::NULL_OBJ_REF)
	{
		result = BooleanObj::FALSE_OBJ_REF;
	}

	if (obj == BooleanObj::TRUE_OBJ_REF)
	{
		result = BooleanObj::TRUE_OBJ_REF;
	}
	
	if(obj == BooleanObj::FALSE_OBJ_REF)
	{
		result = BooleanObj::FALSE_OBJ_REF;
	}

	if (obj->Type() == ObjectType::INTEGER_OBJ)
	{
		auto value = dynamic_cast<IntegerObj*>(obj.get())->Value;
		result = value == 0 ? BooleanObj::FALSE_OBJ_REF : BooleanObj::TRUE_OBJ_REF;
	}

	if (result == nullptr)
	{
		result = MakeError(std::format("illegal expression, type {} can not be evaluated as boolean: {}", obj->Type().Name, obj->Inspect()), context);
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

		auto* ident = dynamic_cast<Identifier*>(param);
		env->Set(ident->Value, args[i]);
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

std::shared_ptr<IObject> StackEvaluator::Eval(const std::shared_ptr<INode>& node, const std::shared_ptr<Environment>& env) const
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

				auto evalBool = EvalAsBoolean(ifex->BaseToken, lastResult);

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

				auto left = results.top();
				results.pop();

				left = UnwrapIfReturnObj(left);

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
			if (signal == 0)
			{
				stack.push({ currentNode, 1, useEnv });
				stack.push({ let->Value.get(), 0, useEnv });
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
			const auto* ident = dynamic_cast<const Identifier*>(currentNode);
			auto value = useEnv->Get(ident->Value);
			if (value != nullptr)
			{
				results.push(value);
			}
			else
			{
				results.push(MakeError(std::format("identifier not found: {}", ident->Value), ident->BaseToken));
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

				if (lastResult->Type() != ObjectType::FUNCTION_OBJ)
				{
					results.push(MakeError(std::format("literal not a function: {}", lastResult->Inspect()), call->BaseToken));
					break;
				}
				auto function = lastResult;

				stack.push({ currentNode, 2, useEnv });

				// Evaluate arguments

				for (const auto& iter : call->Arguments | std::views::reverse)
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

					evalArgs.push_back(arg);
				}
				auto function = results.top();
				results.pop();

				auto func = std::dynamic_pointer_cast<FunctionObj>(function);

				auto extEnv = ExtendFunctionEnv(func, evalArgs);
				stack.push({ func->Body.get(), 0, extEnv });
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

				auto booleanObj = EvalAsBoolean(whileEx->BaseToken, lastResult);

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

				auto booleanObj = EvalAsBoolean(forEx->BaseToken, lastResult);

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


std::shared_ptr<IObject> RecursiveEvaluator::Eval(const std::shared_ptr<INode>& node, const std::shared_ptr<Environment>& env) const
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
		case NodeType::PrefixExpression:
		{
			auto* prefix = dynamic_no_copy_cast<PrefixExpression>(node);
			auto right = Eval(prefix->Right, env);

			right = UnwrapIfReturnObj(right);

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

			if (condition->Type() == ObjectType::ERROR_OBJ)
			{
				return condition;
			}

			auto booleanObj = EvalAsBoolean(ifex->BaseToken, condition);

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

			if (value->Type() == ObjectType::ERROR_OBJ)
			{
				return value;
			}
			env->Set(let->Name->Value, value);
			break;
		}
		case NodeType::Identifier:
		{
			auto* ident = dynamic_no_copy_cast<Identifier>(node);
			auto value = env->Get(ident->Value);
			if (value != nullptr)
			{
				result = value;
			}
			else
			{
				result = MakeError(std::format("identifier not found: {}", ident->Value), ident->BaseToken);
			}
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
			auto function = Eval(call->Function, env);
			if (function->Type() == ObjectType::ERROR_OBJ)
			{
				return function;
			}

			if (function->Type() != ObjectType::FUNCTION_OBJ)
			{
				return MakeError(std::format("literal not a function: {}", function->Inspect()), call->BaseToken);
			}

			auto evalArgs = EvalExpressions(call->Arguments, env);
			if (evalArgs.size() == 1 && evalArgs[0]->Type() == ObjectType::ERROR_OBJ)
			{
				return evalArgs[0];
			}

			auto func = std::dynamic_pointer_cast<FunctionObj>(function);
			result = ApplyFunction(func, evalArgs);
			break;
		}
		case NodeType::WhileExpression:
		{
			auto* whileEx = dynamic_no_copy_cast<WhileExpression>(node);
			auto condition = Eval(whileEx->Condition, env);

			condition = UnwrapIfReturnObj(condition);

			if (condition->Type() == ObjectType::ERROR_OBJ)
			{
				return condition;
			}

			auto booleanObj = EvalAsBoolean(whileEx->BaseToken, condition);

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

				if (condition->Type() == ObjectType::ERROR_OBJ)
				{
					return condition;
				}

				booleanObj = EvalAsBoolean(whileEx->BaseToken, condition);

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

			if (condition->Type() == ObjectType::ERROR_OBJ)
			{
				return condition;
			}

			auto booleanObj = EvalAsBoolean(forEx->BaseToken, condition);

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

				if (condition->Type() == ObjectType::ERROR_OBJ)
				{
					return condition;
				}

				booleanObj = EvalAsBoolean(forEx->BaseToken, condition);

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

std::vector<std::shared_ptr<IObject>> RecursiveEvaluator::EvalExpressions(const std::vector<std::shared_ptr<IExpression>>& expressions, const std::shared_ptr<Environment>& env) const
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

std::shared_ptr<IObject> RecursiveEvaluator::ApplyFunction(const std::shared_ptr<FunctionObj>& func, const std::vector<std::shared_ptr<IObject>>& args) const
{
	auto extEnv = ExtendFunctionEnv(func, args);
	auto evaluated = Eval(func->Body, extEnv);
	return evaluated;
}