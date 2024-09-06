#include "Evaluator.h"

IObject* Evaluator::Eval(Program& program)
{
	IObject* result = nullptr;
	for (const auto& ptr : program.Statements)
	{
		result = Eval(ptr.get());
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
		default:
		{
			result = &NullObj::NULL_OBJ_REF;
		}
	}
		return result;
}
