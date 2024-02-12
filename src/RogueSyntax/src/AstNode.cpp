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
	std::for_each(Statements.begin(), Statements.end(), [&result](const auto& statement)
	{
		result.append(statement->ToString());
	});

	return result;
}

LetStatement::LetStatement(const ::Token token, std::unique_ptr<Identifier> name, std::unique_ptr<IExpression> value) : Token(token)
{
	Name.swap(name);
	Value.swap(value);
}

std::unique_ptr<LetStatement> LetStatement::New(const ::Token token, std::unique_ptr<Identifier> name, std::unique_ptr<IExpression> value)
{
	return std::make_unique<LetStatement>(token, std::move(name), std::move(value));
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

Identifier::Identifier(const ::Token token, const std::string& value) : Token(token), Value(value)
{
}

std::unique_ptr<Identifier> Identifier::New(const ::Token token, const std::string& value)
{
	return std::make_unique<Identifier>(token, value);
}

std::string Identifier::TokenLiteral() const
{
	return Token.Literal;
}

std::string Identifier::ToString() const
{
	return Value;
}

ReturnStatement::ReturnStatement(const ::Token token, std::unique_ptr<IExpression> returnValue) : Token(token)
{
	ReturnValue.swap(returnValue);
}

std::unique_ptr<ReturnStatement> ReturnStatement::New(const ::Token token, std::unique_ptr<IExpression> returnValue)
{
	return std::make_unique<ReturnStatement>(token, std::move(returnValue));
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

ExpressionStatement::ExpressionStatement(const ::Token token, std::unique_ptr<IExpression> expression) : Token(token)
{
	Expression.swap(expression);
}

std::unique_ptr<ExpressionStatement> ExpressionStatement::New(const ::Token token, std::unique_ptr<IExpression> expression)
{
	return std::make_unique<ExpressionStatement>(token, std::move(expression));
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

IntegerLiteral::IntegerLiteral(const ::Token token, int value) : Token(token), Value(value)
{
}

std::unique_ptr<IntegerLiteral> IntegerLiteral::New(::Token token, int value)
{
	return std::make_unique<IntegerLiteral>(token, value);
};

std::string IntegerLiteral::TokenLiteral() const
{
	return Token.Literal;
}

std::string IntegerLiteral::ToString() const
{
	return std::to_string(Value);
}

PrefixExpression::PrefixExpression(const ::Token token, const std::string& op, std::unique_ptr<IExpression> right) : Token(token), Operator(op)
{
	Right.swap(right);
}

std::unique_ptr<PrefixExpression> PrefixExpression::New(const ::Token token, const std::string& op, std::unique_ptr<IExpression> right)
{
	return std::make_unique<PrefixExpression>(token, op, std::move(right));
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

InfixExpression::InfixExpression(const ::Token token, std::unique_ptr<IExpression> left, const std::string& op, std::unique_ptr<IExpression> right) : Token(token), Operator(op)
{
	Left.swap(left);
	Right.swap(right);
}

std::unique_ptr<InfixExpression> InfixExpression::New(const ::Token token, std::unique_ptr<IExpression> left, const std::string& op, std::unique_ptr<IExpression> right)
{
	return std::make_unique<InfixExpression>(token, std::move(left), op, std::move(right));
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