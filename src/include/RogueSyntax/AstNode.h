#pragma once

#include "StandardLib.h"
#include "Token.h"

enum class NodeType
{
	Program,
	Expression,
	Statement,
	Identifier,
	LetStatement,
	ReturnStatement,
	ExpressionStatement,
	IntegerLiteral,
	BooleanLiteral,
	PrefixExpression,
	InfixExpression,
	BlockStatement,
	IfExpression,
	FunctionLiteral,
	CallExpression,
	WhileExpression,
	BreakStatement,
	ContinueStatement,
	ForExpression
};

class INode
{
public:
	virtual std::string ToString() const = 0;
	virtual NodeType NType() const = 0;

	INode(const Token& token) : BaseToken(token) {}
	TokenType Type() const { return BaseToken.Type; };
	std::string TokenLiteral() const { return BaseToken.Literal; };
	Token BaseToken;

	virtual ~INode() = default;
};

class IExpression : public INode
{
public:
	IExpression(const Token& token) : INode(token) {}
	virtual ~IExpression() = default;
};

class IStatement : public INode
{
public:
	IStatement(const Token& token) : INode(token) {}
	virtual ~IStatement() = default;
};

struct Program : IStatement
{
	Program() : IStatement(Token::New(TokenType::TOKEN_ILLEGAL, "")) {};
	std::vector<std::shared_ptr<IStatement>> Statements;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<Program> New();
};

struct Identifier : IExpression
{
	Identifier(const Token token, const std::string& value);
	virtual ~Identifier() = default;
	std::string ToString() const override;
	NodeType NType() const;

	static std::shared_ptr<Identifier> New(const Token token, const std::string& value);


	std::string Value;
};

struct LetStatement : IStatement
{
	LetStatement(const Token token, std::shared_ptr<Identifier> name, std::shared_ptr<IExpression> value);
	virtual ~LetStatement() = default;
	std::string ToString() const override;
	NodeType NType() const override;
	
	static std::shared_ptr<LetStatement> New(const Token token, std::shared_ptr<Identifier> name, std::shared_ptr<IExpression> value);
	std::shared_ptr<Identifier> Name;
	std::shared_ptr<IExpression> Value;
};

struct ReturnStatement : IStatement
{
	ReturnStatement(const Token token, std::shared_ptr<IExpression> returnValue);
	virtual ~ReturnStatement() = default;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<ReturnStatement> New(const Token token, std::shared_ptr<IExpression> returnValue);

	std::shared_ptr<IExpression> ReturnValue;
};

struct ExpressionStatement : IStatement
{
	ExpressionStatement(const Token token, std::shared_ptr<IExpression> expression);
	virtual ~ExpressionStatement() = default;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<ExpressionStatement> New(const Token token, std::shared_ptr<IExpression> expression);

	std::shared_ptr<IExpression> Expression;
};

struct IntegerLiteral : IExpression
{
	IntegerLiteral(const Token token, const int value);
	virtual ~IntegerLiteral() = default;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<IntegerLiteral> New(const Token token, const int value);

	int Value;
};

struct BooleanLiteral : IExpression
{
	BooleanLiteral(const Token token, const bool value);
	virtual ~BooleanLiteral() = default;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<BooleanLiteral> New(const Token token, const bool value);

	bool Value;
};

struct PrefixExpression : IExpression
{
	PrefixExpression(const Token token, const std::string& op, std::shared_ptr<IExpression> right);
	virtual ~PrefixExpression() = default;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<PrefixExpression> New(const Token token, const std::string& op, std::shared_ptr<IExpression> right);

	std::string Operator;
	std::shared_ptr<IExpression> Right;
};

struct InfixExpression : IExpression
{
	InfixExpression(const Token token, std::shared_ptr<IExpression> left, const std::string& op, std::shared_ptr<IExpression> right);
	virtual ~InfixExpression() = default;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<InfixExpression> New(const Token token, std::shared_ptr<IExpression> left, const std::string& op, std::shared_ptr<IExpression> right);

	std::shared_ptr<IExpression> Left;
	std::string Operator;
	std::shared_ptr<IExpression> Right;
};

struct BlockStatement : IStatement
{
	BlockStatement(const Token token, std::vector<std::shared_ptr<IStatement>>& statements);
	virtual ~BlockStatement() = default;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<BlockStatement> New(const Token token, std::vector<std::shared_ptr<IStatement>>& statements);

	std::vector<std::shared_ptr<IStatement>> Statements;
};

struct IfExpression : IExpression
{
	IfExpression(const Token token, std::shared_ptr<IExpression> condition, std::shared_ptr<IStatement> consequence, std::shared_ptr<IStatement> alternative);
	virtual ~IfExpression() = default;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<IfExpression> New(const Token token, std::shared_ptr<IExpression> condition, std::shared_ptr<IStatement> consequence, std::shared_ptr<IStatement> alternative);

	std::shared_ptr<IExpression> Condition;
	std::shared_ptr<IStatement> Consequence;
	std::shared_ptr<IStatement> Alternative;
};

struct FunctionLiteral : IExpression
{
	FunctionLiteral(const Token token, std::vector<std::shared_ptr<IExpression>>& parameters, std::shared_ptr<IStatement> body);
	virtual ~FunctionLiteral() = default;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<FunctionLiteral> New(const Token token, std::vector<std::shared_ptr<IExpression>>& parameters, std::shared_ptr<IStatement> body);

	std::vector<std::shared_ptr<IExpression>> Parameters;
	std::shared_ptr<IStatement> Body;
};

struct CallExpression : IExpression
{
	CallExpression(const Token token, std::shared_ptr<IExpression> function, std::vector<std::shared_ptr<IExpression>>& arguments);
	virtual ~CallExpression() = default;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<CallExpression> New(const Token token, std::shared_ptr<IExpression> function, std::vector<std::shared_ptr<IExpression>>& arguments);

	std::shared_ptr<IExpression> Function;
	std::vector<std::shared_ptr<IExpression>> Arguments;
};

struct WhileExpression : IExpression
{
	WhileExpression(const Token token, std::shared_ptr<IExpression> condition, std::shared_ptr<IStatement> action);
	virtual ~WhileExpression() = default;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<WhileExpression> New(const Token token, std::shared_ptr<IExpression> condition, std::shared_ptr<IStatement> action);

	std::shared_ptr<IExpression> Condition;
	std::shared_ptr<IStatement> Action;
};

struct BreakStatement : IStatement
{
	BreakStatement(const Token token);
	virtual ~BreakStatement() = default;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<BreakStatement> New(const Token token);
};

struct ContinueStatement : IStatement
{
	ContinueStatement(const Token token);
	virtual ~ContinueStatement() = default;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<ContinueStatement> New(const Token token);

};

struct ForExpression : IExpression
{
	ForExpression(const Token token, std::shared_ptr<IStatement> init, std::shared_ptr<IExpression> condition, std::shared_ptr<IStatement> post, std::shared_ptr<IStatement> action);
	virtual ~ForExpression() = default;
	std::string ToString() const override;
	NodeType NType() const override;

	static std::shared_ptr<ForExpression> New(const Token token, std::shared_ptr<IStatement> init, std::shared_ptr<IExpression> condition, std::shared_ptr<IStatement> post, std::shared_ptr<IStatement> action);

	std::shared_ptr<IStatement> Init;
	std::shared_ptr<IExpression> Condition;
	std::shared_ptr<IStatement> Post;
	std::shared_ptr<IStatement> Action;
};