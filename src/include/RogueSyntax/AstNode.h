#pragma once

#include "StandardLib.h"
#include "Identifiable.h"
#include "Token.h"

//forward evaluator
class Evaluator;
class Compiler;

class INode : public IUnquielyIdentifiable
{
public:
	virtual std::string ToString() const = 0;

	INode(const Token& token) : BaseToken(token) {}
	TokenType Type() const { return BaseToken.Type; };
	std::string TokenLiteral() const { return BaseToken.Literal; };
	Token BaseToken;

	virtual void Eval(Evaluator* evaluator) = 0;

	virtual void Compile(Compiler* compiler) = 0;

	virtual ~INode() = default;
};

class IExpression : public INode
{
public:
	IExpression(const Token& token) : INode(token) {}
	virtual ~IExpression() = default;
};

class IStatement : public IExpression
{
public:
	IStatement(const Token& token) : IExpression(token) {}
	virtual ~IStatement() = default;
};

struct Program : IStatement, MetaTypeTag<Program>
{
	Program() : IStatement(Token::New(TokenType::TOKEN_ILLEGAL, "")) { SetUniqueId(this); };
	std::vector<std::shared_ptr<IStatement>> Statements;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);

	static std::shared_ptr<Program> New();
};

struct Identifier : IExpression, MetaTypeTag<Identifier>
{
	Identifier(const Token& token, const std::string& value);
	virtual ~Identifier() = default;
	std::string ToString() const override;

	static std::shared_ptr<Identifier> New(const Token& token, const std::string& value);

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);

	std::string Value;
};

struct LetStatement : IStatement, MetaTypeTag<LetStatement>
{
	LetStatement(const Token& token, const std::shared_ptr<IExpression>& name, const std::shared_ptr<IExpression>& value);
	virtual ~LetStatement() = default;
	std::string ToString() const override;
	
	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);

	static std::shared_ptr<LetStatement> New(const Token& token, const std::shared_ptr<IExpression>& name, const std::shared_ptr<IExpression>& value);
	std::shared_ptr<IExpression> Name;
	std::shared_ptr<IExpression> Value;
};

struct ReturnStatement : IStatement, MetaTypeTag<ReturnStatement>
{
	ReturnStatement(const Token& token, const std::shared_ptr<IExpression>& returnValue);
	virtual ~ReturnStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);

	static std::shared_ptr<ReturnStatement> New(const Token& token, const std::shared_ptr<IExpression>& returnValue);

	std::shared_ptr<IExpression> ReturnValue;
};

struct ExpressionStatement : IStatement, MetaTypeTag<ExpressionStatement>
{
	ExpressionStatement(const Token& token, const std::shared_ptr<IExpression>& expression);
	virtual ~ExpressionStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);

	static std::shared_ptr<ExpressionStatement> New(const Token& token, const std::shared_ptr<IExpression>& expression);

	std::shared_ptr<IExpression> Expression;
};

struct NullLiteral : IExpression, MetaTypeTag<NullLiteral>
{
	NullLiteral(const Token& token);
	virtual ~NullLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);

	static std::shared_ptr<NullLiteral> New(const Token& token);
};

struct IntegerLiteral : IExpression, MetaTypeTag<IntegerLiteral>
{
	IntegerLiteral(const Token& token, const int value);
	virtual ~IntegerLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);

	static std::shared_ptr<IntegerLiteral> New(const Token& token, const int value);

	int Value;
};

struct BooleanLiteral : IExpression, MetaTypeTag<BooleanLiteral>
{
	BooleanLiteral(const Token& token, const bool value);
	virtual ~BooleanLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<BooleanLiteral> New(const Token& token, const bool value);

	bool Value;
};

struct HashLiteral : IExpression, MetaTypeTag<HashLiteral>
{
	HashLiteral(const Token& token, const std::map<std::shared_ptr<IExpression>, std::shared_ptr<IExpression>>& pairs);
	virtual ~HashLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<HashLiteral> New(const Token& token, const std::map<std::shared_ptr<IExpression>, std::shared_ptr<IExpression>>& pairs);

	std::map<std::shared_ptr<IExpression>, std::shared_ptr<IExpression>> Elements;
};

struct PrefixExpression : IExpression, MetaTypeTag<PrefixExpression>
{
	PrefixExpression(const Token& token, const std::string& op, const std::shared_ptr<IExpression>& right);
	virtual ~PrefixExpression() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<PrefixExpression> New(const Token& token, const std::string& op, const std::shared_ptr<IExpression>& right);

	std::string Operator;
	std::shared_ptr<IExpression> Right;
};

struct InfixExpression : IExpression, MetaTypeTag<InfixExpression>
{
	InfixExpression(const Token& token, const std::shared_ptr<IExpression>& left, const std::string& op, const std::shared_ptr<IExpression>& right);
	virtual ~InfixExpression() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<InfixExpression> New(const Token& token, const std::shared_ptr<IExpression>& left, const std::string& op, const std::shared_ptr<IExpression>& right);

	std::shared_ptr<IExpression> Left;
	std::string Operator;
	std::shared_ptr<IExpression> Right;
};

struct BlockStatement : IStatement, MetaTypeTag<BlockStatement>
{
	BlockStatement(const Token& token, const std::vector<std::shared_ptr<IStatement>>& statements);
	virtual ~BlockStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<BlockStatement> New(const Token& token, const std::vector<std::shared_ptr<IStatement>>& statements);

	std::vector<std::shared_ptr<IStatement>> Statements;
};

struct IfStatement : IStatement, MetaTypeTag<IfStatement>
{
	IfStatement(const Token& token, const std::shared_ptr<IExpression>& condition, const std::shared_ptr<IStatement>& consequence, const std::shared_ptr<IStatement>& alternative);
	virtual ~IfStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<IfStatement> New(const Token& token, const std::shared_ptr<IExpression>& condition, const std::shared_ptr<IStatement>& consequence, const std::shared_ptr<IStatement>& alternative);

	std::shared_ptr<IExpression> Condition;
	std::shared_ptr<IStatement> Consequence;
	std::shared_ptr<IStatement> Alternative;
};

struct FunctionLiteral : IExpression, MetaTypeTag<FunctionLiteral>
{
	FunctionLiteral(const Token& token, const std::vector<std::shared_ptr<IExpression>>& parameters, const std::shared_ptr<IStatement>& body);
	virtual ~FunctionLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<FunctionLiteral> New(const Token& token, const std::vector<std::shared_ptr<IExpression>>& parameters, const std::shared_ptr<IStatement>& body);

	std::string Name;
	std::vector<std::shared_ptr<IExpression>> Parameters;
	std::shared_ptr<IStatement> Body;
};

struct CallExpression : IExpression, MetaTypeTag<CallExpression>
{
	CallExpression(const Token& token, const std::shared_ptr<IExpression>& function, const std::vector<std::shared_ptr<IExpression>>& arguments);
	virtual ~CallExpression() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<CallExpression> New(const Token& token, const std::shared_ptr<IExpression>& function, const std::vector<std::shared_ptr<IExpression>>& arguments);

	std::shared_ptr<IExpression> Function;
	std::vector<std::shared_ptr<IExpression>> Arguments;
};

struct WhileStatement : IStatement, MetaTypeTag<WhileStatement>
{
	WhileStatement(const Token& token, const std::shared_ptr<IExpression>& condition, const std::shared_ptr<IStatement>& action);
	virtual ~WhileStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<WhileStatement> New(const Token& token, const std::shared_ptr<IExpression>& condition, const std::shared_ptr<IStatement>& action);

	std::shared_ptr<IExpression> Condition;
	std::shared_ptr<IStatement> Action;
};

struct BreakStatement : IStatement, MetaTypeTag<BreakStatement>
{
	BreakStatement(const Token& token);
	virtual ~BreakStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<BreakStatement> New(const Token& token);
};

struct ContinueStatement : IStatement, MetaTypeTag<ContinueStatement>
{
	ContinueStatement(const Token& token);
	virtual ~ContinueStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<ContinueStatement> New(const Token& token);

};

struct ForStatement : IStatement, MetaTypeTag<ForStatement>
{
	ForStatement(const Token& token, const std::shared_ptr<IStatement>& init, const std::shared_ptr<IExpression>& condition, const std::shared_ptr<IStatement>& post, const std::shared_ptr<IStatement>& action);
	virtual ~ForStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<ForStatement> New(const Token& token, const std::shared_ptr<IStatement>& init, const std::shared_ptr<IExpression>& condition, const std::shared_ptr<IStatement>& post, const std::shared_ptr<IStatement>& action);

	std::shared_ptr<IStatement> Init;
	std::shared_ptr<IExpression> Condition;
	std::shared_ptr<IStatement> Post;
	std::shared_ptr<IStatement> Action;
};

struct StringLiteral : IExpression, MetaTypeTag<StringLiteral>
{
	StringLiteral(const Token& token, const std::string& value);
	virtual ~StringLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<StringLiteral> New(const Token& token, const std::string& value);

	std::string Value;
};

struct DecimalLiteral : IExpression, MetaTypeTag<DecimalLiteral>
{
	DecimalLiteral(const Token& token, const float value);
	virtual ~DecimalLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<DecimalLiteral> New(const Token& token, const float value);

	float Value;
};

struct ArrayLiteral : IExpression, MetaTypeTag<ArrayLiteral>
{
	ArrayLiteral(const Token& token, const std::vector<std::shared_ptr<IExpression>>& elements);
	virtual ~ArrayLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<ArrayLiteral> New(const Token& token, const std::vector<std::shared_ptr<IExpression>>& elements);

	std::vector<std::shared_ptr<IExpression>> Elements;
};

struct IndexExpression : IExpression, MetaTypeTag<IndexExpression>
{
	IndexExpression(const Token& token, const std::shared_ptr<IExpression>& left, const std::shared_ptr<IExpression>& index);
	virtual ~IndexExpression() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator);
	void Compile(Compiler* compiler);
	static std::shared_ptr<IndexExpression> New(const Token& token, const std::shared_ptr<IExpression>& left, const std::shared_ptr<IExpression>& index);

	std::shared_ptr<IExpression> Left;
	std::shared_ptr<IExpression> Index;
};
