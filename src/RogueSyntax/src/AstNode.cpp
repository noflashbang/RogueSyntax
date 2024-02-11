#include "AstNode.h"

std::string Program::TokenLiteral() const
{
	if (Statements.size() > 0)
	{
		return Statements[0]->TokenLiteral();
	}
	else
	{
		return "";
	}
}

std::string Program::ToString() const
{
	std::string result;
	std::for_each(Statements.begin(), Statements.end(), [&result](const auto statement)
	{
		result.append(statement->ToString());
	});

	return result;
}

std::string LetStatement::TokenLiteral() const
{
	return Token.Literal;
}

std::string LetStatement::ToString() const
{
	std::string result;
	result.append(TokenLiteral() + " ");
	result.append(Name->ToString());
	result.append(" = ");
	if (Value != nullptr)
	{
		result.append(Value->ToString());
	}
	result.append(";");
	return result;
}

std::string Identifier::TokenLiteral() const
{
	return Token.Literal;
}

std::string Identifier::ToString() const
{
	return Value;
}

std::string ReturnStatement::TokenLiteral() const
{
	return Token.Literal;
}

std::string ReturnStatement::ToString() const
{
	std::string result;
	result.append(TokenLiteral() + " ");
	if (ReturnValue != nullptr)
	{
		result.append(ReturnValue->ToString());
	}
	result.append(";");
	return result;
}

std::string ExpressionStatement::TokenLiteral() const
{
	return Token.Literal;
}

std::string ExpressionStatement::ToString() const
{
	if (Expression != nullptr)
	{
		return Expression->ToString();
	}
	return "";
}

std::string IntegerLiteral::TokenLiteral() const
{
	return Token.Literal;
}

std::string IntegerLiteral::ToString() const
{
	return std::to_string(Value);
}

std::string PrefixExpression::TokenLiteral() const
{
	return Token.Literal;
}

std::string PrefixExpression::ToString() const
{
	std::string result;
	result.append("(");
	result.append(Operator);
	result.append(Right->ToString());
	result.append(")");
	return result;
}

std::string InfixExpression::TokenLiteral() const
{
	return Token.Literal;
}

std::string InfixExpression::ToString() const
{
	std::string result;
	result.append("(");
	result.append(Left->ToString());
	result.append(" " + Operator + " ");
	result.append(Right->ToString());
	result.append(")");
	return result;
}