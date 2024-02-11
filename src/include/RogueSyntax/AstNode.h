#pragma once

#include "StandardLib.h"
#include "Token.h"

class INode
{
public:
	virtual std::string TokenLiteral() const = 0;
	virtual std::string ToString() const = 0;
};

class IExpression : public INode
{
public:
	//virtual std::string TokenLiteral() const = 0;
};

class IStatement : public INode
{
public:
	//virtual std::string TokenLiteral() const = 0;
};


struct Program
{
	std::vector<IStatement*> Statements;

	std::string TokenLiteral() const;
	std::string ToString() const;
};

struct Identifier : IExpression
{
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;

	Token Token;
	std::string Value;
};

struct LetStatement : IStatement
{
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;
	
	Token Token;
	Identifier* Name;
	IExpression* Value;
};

struct ReturnStatement : IStatement
{
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;

	Token Token;
	IExpression* ReturnValue;
};

struct ExpressionStatement : IStatement
{
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;

	Token Token;
	IExpression* Expression;
};

struct IntegerLiteral : IExpression
{
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;

	Token Token;
	int Value;
};

struct PrefixExpression : IExpression
{
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;

	Token Token;
	std::string Operator;
	IExpression* Right;
};

struct InfixExpression : IExpression
{
	virtual std::string TokenLiteral() const override;
	virtual std::string ToString() const override;

	Token Token;
	IExpression* Left;
	std::string Operator;
	IExpression* Right;
};