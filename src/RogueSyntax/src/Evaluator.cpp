#include "Evaluator.h"

IObject* Evaluator::Eval(Program& program)
{
	IObject* result = nullptr;
	for (const auto& stmt : program.Statements)
	{
		result = Eval(stmt.get());
		if (result->Type() == ObjectType::RETURN_OBJ)
		{
			result = dynamic_cast<ReturnObj*>(result)->Value;
			break;
		}
		if (result->Type() == ObjectType::ERROR_OBJ)
		{
			break;
		}
	}
	return result;
}

IObject* Evaluator::Eval(INode* node)
{
	auto type = node->NType();
	IObject* result = nullptr;
	switch (type)
	{
		case NodeType::Program:
		{
			auto program = dynamic_cast<Program*>(node);
			result = Eval(*program);
			break;
		}
		case NodeType::ExpressionStatement:
		{
			auto expression = dynamic_cast<ExpressionStatement*>(node);
			result = Eval(expression->Expression.get());
			break;
		}
		case NodeType::IntegerLiteral:
		{
			auto integer = dynamic_cast<IntegerLiteral*>(node);
			result = new IntegerObj(integer->Value);
			break;
		}
		case NodeType::BooleanLiteral:
		{
			auto boolean = dynamic_cast<BooleanLiteral*>(node);
			result = boolean->Value ? &BooleanObj::TRUE_OBJ_REF : &BooleanObj::FALSE_OBJ_REF;
			break;
		}
		case NodeType::PrefixExpression:
		{
			auto prefix = dynamic_cast<PrefixExpression*>(node);
			auto right = Eval(prefix->Right.get());
			if (right->Type() == ObjectType::ERROR_OBJ)
			{
				return right;
			}
			result = EvalPrefixExpression(prefix->Token, right);
			break;
		}
		case NodeType::BlockStatement:
		{
			auto block = dynamic_cast<BlockStatement*>(node);
			for (const auto& stmt : block->Statements)
			{
				result = Eval(stmt.get());
				if (result->Type() == ObjectType::RETURN_OBJ || result->Type() == ObjectType::ERROR_OBJ)
				{
					break;
				}
			}
			break;
		}
		case NodeType::IfExpression:
		{
			auto ifex = dynamic_cast<IfExpression*>(node);

			auto condition = Eval(ifex->Condition.get());
			if (condition->Type() == ObjectType::ERROR_OBJ)
			{
				return condition;
			}

			if (IsTruthy(condition))
			{
				result = Eval(ifex->Consequence.get());
			}
			else if (ifex->Alternative != nullptr)
			{
				result = Eval(ifex->Alternative.get());
			}
			else
			{
				result = &NullObj::NULL_OBJ_REF;
			}
			break;
		}
		case NodeType::InfixExpression:
		{
			auto infix = dynamic_cast<InfixExpression*>(node);
			auto left = Eval(infix->Left.get());
			if (left->Type() == ObjectType::ERROR_OBJ)
			{
				return left;
			}
			auto right = Eval(infix->Right.get());
			if (right->Type() == ObjectType::ERROR_OBJ)
			{
				return right;
			}
			result = EvalInfixExpression(infix->Token, left, right);
			break;
		}
		case NodeType::ReturnStatement:
		{
			auto ret = dynamic_cast<ReturnStatement*>(node);
			auto value = Eval(ret->ReturnValue.get());
			result = new ReturnObj(value);
			break;
		}
		default:
		{
			result = &NullObj::NULL_OBJ_REF;
		}
	}
		return result;
}

IObject* Evaluator::EvalPrefixExpression(const Token& op, IObject* right)
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

IObject* Evaluator::EvalInfixExpression(const Token& op, IObject* left, IObject* right)
{
	if (left->Type() != right->Type())
	{
		return MakeError(std::format("type mismatch: {} {} {}", left->Type().Name, op.Literal, right->Type().Name), op);
	}

	if (left->Type() == ObjectType::INTEGER_OBJ && right->Type() == ObjectType::INTEGER_OBJ)
	{
		return EvalIntegerInfixExpression(op, dynamic_cast<IntegerObj*>(left), dynamic_cast<IntegerObj*>(right));
	}
	else if (left->Type() == ObjectType::BOOLEAN_OBJ && right->Type() == ObjectType::BOOLEAN_OBJ)
	{
		return EvalBooleanInfixExpression(op, dynamic_cast<BooleanObj*>(left), dynamic_cast<BooleanObj*>(right));
	}
	else
	{
		return MakeError(std::format("unknown operator: {} {} {}", left->Type().Name, op.Literal, right->Type().Name), op);
	}
}

bool Evaluator::IsTruthy(IObject* obj)
{
	auto booleanObj = EvalAsBoolean(obj);
	if (booleanObj == &BooleanObj::TRUE_OBJ_REF)
	{
		return true;
	}
	else
	{
		return false;
	}
}

BooleanObj* Evaluator::EvalAsBoolean(IObject* obj)
{
	if (obj == &NullObj::NULL_OBJ_REF)
	{
		return &BooleanObj::FALSE_OBJ_REF;
	}

	if (obj == &BooleanObj::TRUE_OBJ_REF)
	{
		return &BooleanObj::TRUE_OBJ_REF;
	}
	
	if(obj == &BooleanObj::FALSE_OBJ_REF)
	{
		return &BooleanObj::FALSE_OBJ_REF;
	}

	if (obj->Type() == ObjectType::INTEGER_OBJ)
	{
		auto value = dynamic_cast<IntegerObj*>(obj)->Value;
		return value == 0 ? &BooleanObj::FALSE_OBJ_REF : &BooleanObj::TRUE_OBJ_REF;
	}
}

IObject* Evaluator::EvalIntegerInfixExpression(const Token& op, IntegerObj* left, IntegerObj* right)
{
	if (op.Type == TokenType::TOKEN_PLUS)
	{
		return new IntegerObj(left->Value + right->Value);
	}
	else if (op.Type == TokenType::TOKEN_MINUS)
	{
		return new IntegerObj(left->Value - right->Value);
	}
	else if (op.Type == TokenType::TOKEN_ASTERISK)
	{
		return new IntegerObj(left->Value * right->Value);
	}
	else if (op.Type == TokenType::TOKEN_SLASH)
	{
		return new IntegerObj(left->Value / right->Value);
	}
	else if (op.Type == TokenType::TOKEN_LT)
	{
		return left->Value < right->Value ? &BooleanObj::TRUE_OBJ_REF : &BooleanObj::FALSE_OBJ_REF;
	}
	else if (op.Type == TokenType::TOKEN_GT)
	{
		return left->Value > right->Value ? &BooleanObj::TRUE_OBJ_REF : &BooleanObj::FALSE_OBJ_REF;
	}
	else if (op.Type == TokenType::TOKEN_EQ)
	{
		return left->Value == right->Value ? &BooleanObj::TRUE_OBJ_REF : &BooleanObj::FALSE_OBJ_REF;
	}
	else if (op.Type == TokenType::TOKEN_NOT_EQ)
	{
		return left->Value != right->Value ? &BooleanObj::TRUE_OBJ_REF : &BooleanObj::FALSE_OBJ_REF;
	}
	else
	{
		return MakeError(std::format("unknown operator: {} {} {}", left->Type().Name, op.Literal, right->Type().Name), op);
	}
}

IObject* Evaluator::EvalBooleanInfixExpression(const Token& op, BooleanObj* left, BooleanObj* right)
{
	if (op.Type == TokenType::TOKEN_EQ)
	{
		return left->Value == right->Value ? &BooleanObj::TRUE_OBJ_REF : &BooleanObj::FALSE_OBJ_REF;
	}
	else if (op.Type == TokenType::TOKEN_NOT_EQ)
	{
		return left->Value != right->Value ? &BooleanObj::TRUE_OBJ_REF : &BooleanObj::FALSE_OBJ_REF;
	}
	else
	{
		return MakeError(std::format("unknown operator: {} {} {}", left->Type().Name, op.Literal, right->Type().Name), op);
	}
}

IObject* Evaluator::EvalBangOperatorExpression(const Token& op, IObject* right)
{
	if (right == &BooleanObj::TRUE_OBJ_REF)
	{
		return &BooleanObj::FALSE_OBJ_REF;
	}
	else if (right == &BooleanObj::FALSE_OBJ_REF)
	{
		return &BooleanObj::TRUE_OBJ_REF;
	}
	else if (right == &NullObj::NULL_OBJ_REF)
	{
		return &BooleanObj::TRUE_OBJ_REF;
	}
	else if (right->Type() == ObjectType::INTEGER_OBJ)
	{
		auto value = dynamic_cast<IntegerObj*>(right)->Value;
		if (value == 0)
		{
			return &BooleanObj::TRUE_OBJ_REF;
		}
		else
		{
			return &BooleanObj::FALSE_OBJ_REF;
		}
	}
	else
	{
		return MakeError(std::format("unknown operator: {}{}", op.Literal, right->Type().Name), op);
	}
}

IObject* Evaluator::EvalMinusPrefixOperatorExpression(const Token& op, IObject* right)
{
	if (right->Type() != ObjectType::INTEGER_OBJ)
	{
		return MakeError(std::format("unknown operator: {}{}", op.Literal, right->Type().Name), op);
	}

	auto value = dynamic_cast<IntegerObj*>(right)->Value;
	return new IntegerObj(-value);
}

IObject* Evaluator::MakeError(const std::string& message, const Token& token)
{
	return new ErrorObj(message, token);
}