#pragma once

#include "StandardLib.h"
#include "Token.h"

class INode
{
public:
	virtual std::string TokenLiteral() const = 0;
	virtual std::string ToString() const = 0;
	virtual TokenType Type() const = 0;

	virtual ~INode() = default;
};

class IExpression : public INode
{
public:
	virtual ~IExpression() = default;
};

class IStatement : public INode
{
public:
	virtual ~IStatement() = default;
};


struct Program
{
	std::vector<std::unique_ptr<IStatement>> Statements;
	std::string TokenLiteral() const;
	std::string ToString() const;
};

struct Identifier : IExpression
{
	Identifier(const Token token, const std::string& value);
	virtual ~Identifier() = default;
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;
	virtual TokenType Type() const;

	static std::unique_ptr<Identifier> New(const Token token, const std::string& value);

	Token Token;
	std::string Value;
};

struct LetStatement : IStatement
{
	LetStatement(const Token token, std::unique_ptr<Identifier> name, std::unique_ptr<IExpression> value);
	virtual ~LetStatement() = default;
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;
	virtual TokenType Type() const;
	
	static std::unique_ptr<LetStatement> New(const Token token, std::unique_ptr<Identifier> name, std::unique_ptr<IExpression> value);

	Token Token;
	std::unique_ptr<Identifier> Name;
	std::unique_ptr<IExpression> Value;
};

struct ReturnStatement : IStatement
{
	ReturnStatement(const Token token, std::unique_ptr<IExpression> returnValue);
	virtual ~ReturnStatement() = default;
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;
	virtual TokenType Type() const;

	static std::unique_ptr<ReturnStatement> New(const Token token, std::unique_ptr<IExpression> returnValue);

	Token Token;
	std::unique_ptr<IExpression> ReturnValue;
};

struct ExpressionStatement : IStatement
{
	ExpressionStatement(const Token token, std::unique_ptr<IExpression> expression);
	virtual ~ExpressionStatement() = default;
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;
	virtual TokenType Type() const;

	static std::unique_ptr<ExpressionStatement> New(const Token token, std::unique_ptr<IExpression> expression);

	Token Token;
	std::unique_ptr<IExpression> Expression;
};

struct IntegerLiteral : IExpression
{
	IntegerLiteral(const Token token, const int value);
	virtual ~IntegerLiteral() = default;
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;
	virtual TokenType Type() const;

	static std::unique_ptr<IntegerLiteral> New(const Token token, const int value);

	Token Token;
	int Value;
};

struct BooleanLiteral : IExpression
{
	BooleanLiteral(const Token token, const bool value);
	virtual ~BooleanLiteral() = default;
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;
	virtual TokenType Type() const;

	static std::unique_ptr<BooleanLiteral> New(const Token token, const bool value);

	Token Token;
	bool Value;
};

struct PrefixExpression : IExpression
{
	PrefixExpression(const Token token, const std::string& op, std::unique_ptr<IExpression> right);
	virtual ~PrefixExpression() = default;
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;
	virtual TokenType Type() const;

	static std::unique_ptr<PrefixExpression> New(const Token token, const std::string& op, std::unique_ptr<IExpression> right);

	Token Token;
	std::string Operator;
	std::unique_ptr<IExpression> Right;
};

struct InfixExpression : IExpression
{
	InfixExpression(const Token token, std::unique_ptr<IExpression> left, const std::string& op, std::unique_ptr<IExpression> right);
	virtual ~InfixExpression() = default;
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;
	virtual TokenType Type() const;

	static std::unique_ptr<InfixExpression> New(const Token token, std::unique_ptr<IExpression> left, const std::string& op, std::unique_ptr<IExpression> right);

	Token Token;
	std::unique_ptr<IExpression> Left;
	std::string Operator;
	std::unique_ptr<IExpression> Right;
};

struct BlockStatement : IStatement
{
	BlockStatement(const Token token, std::vector<std::unique_ptr<IStatement>>& statements);
	virtual ~BlockStatement() = default;
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;
	virtual TokenType Type() const;

	static std::unique_ptr<BlockStatement> New(const Token token, std::vector<std::unique_ptr<IStatement>>& statements);

	Token Token;
	std::vector<std::unique_ptr<IStatement>> Statements;
};

struct IfExpression : IExpression
{
	IfExpression(const Token token, std::unique_ptr<IExpression> condition, std::unique_ptr<IStatement> consequence, std::unique_ptr<IStatement> alternative);
	virtual ~IfExpression() = default;
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;
	virtual TokenType Type() const;

	static std::unique_ptr<IfExpression> New(const Token token, std::unique_ptr<IExpression> condition, std::unique_ptr<IStatement> consequence, std::unique_ptr<IStatement> alternative);

	Token Token;
	std::unique_ptr<IExpression> Condition;
	std::unique_ptr<IStatement> Consequence;
	std::unique_ptr<IStatement> Alternative;
};

struct FunctionLiteral : IExpression
{
	FunctionLiteral(const Token token, std::vector<std::unique_ptr<IExpression>>& parameters, std::unique_ptr<IStatement> body);
	virtual ~FunctionLiteral() = default;
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;
	virtual TokenType Type() const;

	static std::unique_ptr<FunctionLiteral> New(const Token token, std::vector<std::unique_ptr<IExpression>>& parameters, std::unique_ptr<IStatement> body);

	Token Token;
	std::vector<std::unique_ptr<IExpression>> Parameters;
	std::unique_ptr<IStatement> Body;
};

struct CallExpression : IExpression
{
	CallExpression(const Token token, std::unique_ptr<IExpression> function, std::vector<std::unique_ptr<IExpression>>& arguments);
	virtual ~CallExpression() = default;
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;
	virtual TokenType Type() const;

	static std::unique_ptr<CallExpression> New(const Token token, std::unique_ptr<IExpression> function, std::vector<std::unique_ptr<IExpression>>& arguments);

	Token Token;
	std::unique_ptr<IExpression> Function;
	std::vector<std::unique_ptr<IExpression>> Arguments;
};