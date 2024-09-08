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

LetStatement::LetStatement(const ::Token token, std::shared_ptr<Identifier> name, std::shared_ptr<IExpression> value) : Token(token)
{
	Name = name;
	Value = value;
}

std::shared_ptr<LetStatement> LetStatement::New(const ::Token token, std::shared_ptr<Identifier> name, std::shared_ptr<IExpression> value)
{
	return std::make_shared<LetStatement>(token, name, value);
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

std::shared_ptr<Identifier> Identifier::New(const ::Token token, const std::string& value)
{
	return std::make_shared<Identifier>(token, value);
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

ReturnStatement::ReturnStatement(const ::Token token, std::shared_ptr<IExpression> returnValue) : Token(token)
{
	ReturnValue = returnValue;
}

std::shared_ptr<ReturnStatement> ReturnStatement::New(const ::Token token, std::shared_ptr<IExpression> returnValue)
{
	return std::make_shared<ReturnStatement>(token, returnValue);
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

ExpressionStatement::ExpressionStatement(const ::Token token, std::shared_ptr<IExpression> expression) : Token(token)
{
	Expression = expression;
}

std::shared_ptr<ExpressionStatement> ExpressionStatement::New(const ::Token token, std::shared_ptr<IExpression> expression)
{
	return std::make_shared<ExpressionStatement>(token,  expression);
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

std::shared_ptr<IntegerLiteral> IntegerLiteral::New(::Token token, int value)
{
	return std::make_shared<IntegerLiteral>(token, value);
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

std::shared_ptr<BooleanLiteral> BooleanLiteral::New(::Token token, bool value)
{
	return std::make_shared<BooleanLiteral>(token, value);
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

PrefixExpression::PrefixExpression(const ::Token token, const std::string& op, std::shared_ptr<IExpression> right) : Token(token), Operator(op)
{
	Right = right;
}

std::shared_ptr<PrefixExpression> PrefixExpression::New(const ::Token token, const std::string& op, std::shared_ptr<IExpression> right)
{
	return std::make_shared<PrefixExpression>(token, op, right);
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

InfixExpression::InfixExpression(const ::Token token, std::shared_ptr<IExpression> left, const std::string& op, std::shared_ptr<IExpression> right) : Token(token), Operator(op)
{
	Left = left;
	Right = right;
}

std::shared_ptr<InfixExpression> InfixExpression::New(const ::Token token, std::shared_ptr<IExpression> left, const std::string& op, std::shared_ptr<IExpression> right)
{
	return std::make_shared<InfixExpression>(token, left, op, right);
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

BlockStatement::BlockStatement(const ::Token token, std::vector<std::shared_ptr<IStatement>>& statements) : Token(token)
{
	Statements = statements;
}

std::shared_ptr<BlockStatement> BlockStatement::New(const ::Token token, std::vector<std::shared_ptr<IStatement>>& statements)
{
	return std::make_shared<BlockStatement>(token, statements);
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

IfExpression::IfExpression(const ::Token token, std::shared_ptr<IExpression> condition, std::shared_ptr<IStatement> consequence, std::shared_ptr<IStatement> alternative) : Token(token)
{
	Condition = condition;
	Consequence = consequence;
	Alternative = alternative;
}

std::shared_ptr<IfExpression> IfExpression::New(const ::Token token, std::shared_ptr<IExpression> condition, std::shared_ptr<IStatement> consequence, std::shared_ptr<IStatement> alternative)
{
	return std::make_shared<IfExpression>(token, condition, consequence, alternative);
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

FunctionLiteral::FunctionLiteral(const ::Token token, std::vector<std::shared_ptr<IExpression>>& parameters, std::shared_ptr<IStatement> body) : Token(token)
{
	Parameters = parameters;
	Body = body;
}

std::shared_ptr<FunctionLiteral> FunctionLiteral::New(const ::Token token, std::vector<std::shared_ptr<IExpression>>& parameters, std::shared_ptr<IStatement> body)
{
	return std::make_shared<FunctionLiteral>(token, parameters, body);
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

CallExpression::CallExpression(const ::Token token, std::shared_ptr<IExpression> function, std::vector<std::shared_ptr<IExpression>>& arguments) : Token(token)
{
	Function = function;
	Arguments= arguments;
}

std::shared_ptr<CallExpression> CallExpression::New(const ::Token token, std::shared_ptr<IExpression> function, std::vector<std::shared_ptr<IExpression>>& arguments)
{
	return std::make_shared<CallExpression>(token, function, arguments);
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

WhileExpression::WhileExpression(const ::Token token, std::shared_ptr<IExpression> condition, std::shared_ptr<IStatement> action) : Token(token)
{
	Condition = condition;
	Action = action;
}

std::shared_ptr<WhileExpression> WhileExpression::New(const ::Token token, std::shared_ptr<IExpression> condition, std::shared_ptr<IStatement> action)
{
	return std::make_shared<WhileExpression>(token, condition, action);
}

std::string WhileExpression::TokenLiteral() const
{
	return Token.Literal;
}

std::string WhileExpression::ToString() const
{
	std::string result;
	result.append("while");
	result.append(Condition->ToString());
	result.append(" ");
	result.append(Action->ToString());
	return result;
}

TokenType WhileExpression::Type() const
{
	return Token.Type;
}

NodeType WhileExpression::NType() const
{
	return NodeType::WhileExpression;
}


BreakStatement::BreakStatement(const ::Token token) : Token(token)
{
}

std::shared_ptr<BreakStatement> BreakStatement::New(const ::Token token)
{
	return std::make_shared<BreakStatement>(token);
}

std::string BreakStatement::TokenLiteral() const
{
	return Token.Literal;
}

std::string BreakStatement::ToString() const
{
	return "break";
}

TokenType BreakStatement::Type() const
{
	return Token.Type;
}

NodeType BreakStatement::NType() const
{
	return NodeType::BreakStatement;
}

ContinueStatement::ContinueStatement(const ::Token token) : Token(token)
{
}

std::shared_ptr<ContinueStatement> ContinueStatement::New(const ::Token token)
{
	return std::make_shared<ContinueStatement>(token);
}

std::string ContinueStatement::TokenLiteral() const
{
	return Token.Literal;
}

std::string ContinueStatement::ToString() const
{
	return "continue";
}

TokenType ContinueStatement::Type() const
{
	return Token.Type;
}

NodeType ContinueStatement::NType() const
{
	return NodeType::ContinueStatement;
}

ForExpression::ForExpression(const ::Token token, std::shared_ptr<IStatement> init, std::shared_ptr<IExpression> condition, std::shared_ptr<IStatement> post, std::shared_ptr<IStatement> action) : Token(token)
{
	Init = init;
	Condition = condition;
	Post = post;
	Action = action;
}

std::shared_ptr<ForExpression> ForExpression::New(const ::Token token, std::shared_ptr<IStatement> init, std::shared_ptr<IExpression> condition, std::shared_ptr<IStatement> post, std::shared_ptr<IStatement> action)
{
	return std::make_shared<ForExpression>(token, init, condition, post, action);
}

std::string ForExpression::TokenLiteral() const
{
	return Token.Literal;
}

std::string ForExpression::ToString() const
{
	std::string result;
	result.append("for (");
	result.append(Init->ToString());
	result.append(" ");
	result.append(Condition->ToString());
	result.append("; ");
	result.append(Post->ToString());
	if (result.ends_with(";"))
	{
		result.pop_back();
	}
	result.append(") ");
	result.append(Action->ToString());
	return result;
}

TokenType ForExpression::Type() const
{
	return Token.Type;
}

NodeType ForExpression::NType() const
{
	return NodeType::ForExpression;
}
