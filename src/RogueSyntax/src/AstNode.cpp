#include "AstNode.h"
#include "AstNode.h"
#include "AstNode.h"
#include "AstNode.h"
#include "AstNode.h"
#include "AstNode.h"
#include "AstNode.h"
#include "AstNode.h"

TokenType Program::Type() const
{
	return TokenType::TOKEN_ILLEGAL;
}
NodeType Program::NType() const
{
	return NodeType::Program;
}

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

TokenType LetStatement::Type() const
{
	return Token.Type;
}

NodeType LetStatement::NType() const
{
	return NodeType::LetStatement;
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

TokenType Identifier::Type() const
{
	return Token.Type;
}

NodeType Identifier::NType() const
{
	return NodeType::Identifier;
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

TokenType ReturnStatement::Type() const
{
	return Token.Type;
}

NodeType ReturnStatement::NType() const
{
	return NodeType::ReturnStatement;
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

TokenType ExpressionStatement::Type() const
{
	return Token.Type;
}

NodeType ExpressionStatement::NType() const
{
	return NodeType::ExpressionStatement;
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

TokenType IntegerLiteral::Type() const
{
	return Token.Type;
}

NodeType IntegerLiteral::NType() const
{
	return NodeType::IntegerLiteral;
}

BooleanLiteral::BooleanLiteral(const ::Token token, bool value) : Token(token), Value(value)
{
}

std::unique_ptr<BooleanLiteral> BooleanLiteral::New(::Token token, bool value)
{
	return std::make_unique<BooleanLiteral>(token, value);
};

std::string BooleanLiteral::TokenLiteral() const
{
	return Token.Literal;
}

std::string BooleanLiteral::ToString() const
{
	return Value ? "true" : "false";
}

TokenType BooleanLiteral::Type() const
{
	return Token.Type;
}

NodeType BooleanLiteral::NType() const
{
	return NodeType::BooleanLiteral;
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

TokenType PrefixExpression::Type() const
{
	return Token.Type;
}

NodeType PrefixExpression::NType() const
{
	return NodeType::PrefixExpression;
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

TokenType InfixExpression::Type() const
{
	return Token.Type;
}

NodeType InfixExpression::NType() const
{
	return NodeType::InfixExpression;
}

BlockStatement::BlockStatement(const ::Token token, std::vector<std::unique_ptr<IStatement>>& statements) : Token(token)
{
	Statements.swap(statements);
}

std::unique_ptr<BlockStatement> BlockStatement::New(const ::Token token, std::vector<std::unique_ptr<IStatement>>& statements)
{
	return std::make_unique<BlockStatement>(token, statements);
}

std::string BlockStatement::TokenLiteral() const
{
	return Token.Literal;
}

std::string BlockStatement::ToString() const
{
	std::string result;
	result.append("{");
	std::for_each(Statements.begin(), Statements.end(), [&result](const auto& statement)
	{
		result.append(statement->ToString());
	});
	result.append("}");

	return result;
}

TokenType BlockStatement::Type() const
{
	return Token.Type;
}

NodeType BlockStatement::NType() const
{
	return NodeType::BlockStatement;
}

IfExpression::IfExpression(const ::Token token, std::unique_ptr<IExpression> condition, std::unique_ptr<IStatement> consequence, std::unique_ptr<IStatement> alternative) : Token(token)
{
	Condition.swap(condition);
	Consequence.swap(consequence);
	Alternative.swap(alternative);
}

std::unique_ptr<IfExpression> IfExpression::New(const ::Token token, std::unique_ptr<IExpression> condition, std::unique_ptr<IStatement> consequence, std::unique_ptr<IStatement> alternative)
{
	return std::make_unique<IfExpression>(token, std::move(condition), std::move(consequence), std::move(alternative));
}

std::string IfExpression::TokenLiteral() const
{
	return Token.Literal;
}

std::string IfExpression::ToString() const
{
	std::string result;
	result.append("if");
	result.append(Condition->ToString());
	result.append(" ");
	result.append(Consequence->ToString());
	if (Alternative != nullptr)
	{
		result.append("else ");
		result.append(Alternative->ToString());
	}
	return result;
}

TokenType IfExpression::Type() const
{
	return Token.Type;
}

NodeType IfExpression::NType() const
{
	return NodeType::IfExpression;
}

FunctionLiteral::FunctionLiteral(const ::Token token, std::vector<std::unique_ptr<IExpression>>& parameters, std::unique_ptr<IStatement> body) : Token(token)
{
	Parameters.swap(parameters);
	Body.swap(body);
}

std::unique_ptr<FunctionLiteral> FunctionLiteral::New(const ::Token token, std::vector<std::unique_ptr<IExpression>>& parameters, std::unique_ptr<IStatement> body)
{
	return std::make_unique<FunctionLiteral>(token, parameters, std::move(body));
}

std::string FunctionLiteral::TokenLiteral() const
{
	return Token.Literal;
}

std::string FunctionLiteral::ToString() const
{
	std::string result;
	result.append(TokenLiteral());
	result.append("(");
	std::for_each(Parameters.begin(), Parameters.end(), [&result](const auto& param)
	{
		result.append(param->ToString());
		result.append(", ");
	});

	//remove the last comma
	result.pop_back();
	result.pop_back();

	result.append(")");
	result.append(Body->ToString());
	return result;
}

TokenType FunctionLiteral::Type() const
{
	return Token.Type;
}

NodeType FunctionLiteral::NType() const
{
	return NodeType::FunctionLiteral;
}

CallExpression::CallExpression(const ::Token token, std::unique_ptr<IExpression> function, std::vector<std::unique_ptr<IExpression>>& arguments) : Token(token)
{
	Function.swap(function);
	Arguments.swap(arguments);
}

std::unique_ptr<CallExpression> CallExpression::New(const ::Token token, std::unique_ptr<IExpression> function, std::vector<std::unique_ptr<IExpression>>& arguments)
{
	return std::make_unique<CallExpression>(token, std::move(function), arguments);
}

std::string CallExpression::TokenLiteral() const
{
	return Token.Literal;
}

std::string CallExpression::ToString() const
{
	std::string result;
	result.append(Function->ToString());
	result.append("(");
	std::for_each(Arguments.begin(), Arguments.end(), [&result](const auto& arg)
	{
		result.append(arg->ToString());
		result.append(", ");
	});

	//remove the last comma
	result.pop_back();
	result.pop_back();

	result.append(")");
	return result;
}

TokenType CallExpression::Type() const
{
	return Token.Type;
}

NodeType CallExpression::NType() const
{
	return NodeType::CallExpression;
}
