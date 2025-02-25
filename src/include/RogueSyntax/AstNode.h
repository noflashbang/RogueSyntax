#pragma once

#include "StandardLib.h"
#include "Identifiable.h"
#include "Token.h"

//forward evaluator
class Evaluator;
class Compiler;
class AstNodeStore;

class INode : public IUnquielyIdentifiable
{
public:
	virtual std::string ToString() const = 0;

	INode(const RSToken& token) : BaseToken(token) {}
	TokenType Type() const { return BaseToken.Type; };
	std::string TokenLiteral() const { return BaseToken.Literal; };
	RSToken BaseToken;

	virtual void Eval(Evaluator* evaluator) const = 0;
	virtual void Compile(Compiler* compiler) const = 0;

	virtual ~INode() = default;
};

class IExpression : public INode
{
public:
	IExpression(const RSToken& token) : INode(token) {}
	virtual ~IExpression() = default;
};

class IStatement : public IExpression
{
public:
	IStatement(const RSToken& token) : IExpression(token) {}
	virtual ~IStatement() = default;
};

struct Program : IStatement
{
	Program(const std::shared_ptr<AstNodeStore>& store, const std::string& unitName) : IStatement(RSToken::New(TokenType::TOKEN_ILLEGAL, "")), _store(store), _unitName(unitName) { SetUniqueId(this); };
	std::vector<IStatement*> Statements;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;

	static std::shared_ptr<Program> New(const std::shared_ptr<AstNodeStore>& store, const std::string& unitName);
private:
	std::shared_ptr<AstNodeStore> _store;
	const std::string& _unitName;
};

struct Identifier : IExpression
{
	Identifier(const RSToken& token, const std::string& value);
	virtual ~Identifier() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;

	std::string Value;
};

struct LetStatement : IStatement
{
	LetStatement(const RSToken& token, const IExpression* name, const IExpression* value);
	virtual ~LetStatement() = default;
	std::string ToString() const override;
	
	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;

	const IExpression* Name;
	const IExpression* Value;
};

struct ReturnStatement : IStatement
{
	ReturnStatement(const RSToken& token, const IExpression* returnValue);
	virtual ~ReturnStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;

	const IExpression* ReturnValue;
};

struct ExpressionStatement : IStatement
{
	ExpressionStatement(const RSToken& token, const IExpression* expression);
	virtual ~ExpressionStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;

	const IExpression* Expression;
};

struct NullLiteral : IExpression
{
	NullLiteral(const RSToken& token);
	virtual ~NullLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
};

struct IntegerLiteral : IExpression
{
	IntegerLiteral(const RSToken& token, const int value);
	virtual ~IntegerLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;

	int Value;
};

struct BooleanLiteral : IExpression
{
	BooleanLiteral(const RSToken& token, const bool value);
	virtual ~BooleanLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
	
	bool Value;
};

struct HashLiteral : IExpression
{
	HashLiteral(const RSToken& token, const std::map<IExpression*, IExpression*>& pairs);
	virtual ~HashLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
	
	std::map<IExpression*, IExpression*> Elements;
};

struct PrefixExpression : IExpression
{
	PrefixExpression(const RSToken& token, const std::string& op, const IExpression* right);
	virtual ~PrefixExpression() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
	

	std::string Operator;
	const IExpression* Right;
};

struct InfixExpression : IExpression
{
	InfixExpression(const RSToken& token, const IExpression* left, const std::string& op, const IExpression* right);
	virtual ~InfixExpression() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
	
	const IExpression* Left;
	std::string Operator;
	const IExpression* Right;
};

struct BlockStatement : IStatement
{
	BlockStatement(const RSToken& token, const std::vector<IStatement*>& statements);
	virtual ~BlockStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
	
	std::vector<IStatement*> Statements;
};

struct IfStatement : IStatement
{
	IfStatement(const RSToken& token, const IExpression* condition, const IStatement* consequence, const IStatement* alternative);
	virtual ~IfStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
	
	const IExpression* Condition;
	const IStatement* Consequence;
	const IStatement* Alternative;
};

struct FunctionLiteral : IExpression
{
	FunctionLiteral(const RSToken& token, const std::vector<IExpression*>& parameters, const IStatement* body);
	virtual ~FunctionLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
	
	std::string Name;
	std::vector<IExpression*> Parameters;
	const IStatement* Body;
};

struct CallExpression : IExpression
{
	CallExpression(const RSToken& token, const IExpression* function, const std::vector<IExpression*>& arguments);
	virtual ~CallExpression() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
	
	const IExpression* Function;
	std::vector<IExpression*> Arguments;
};

struct WhileStatement : IStatement
{
	WhileStatement(const RSToken& token, const IExpression* condition, const IStatement* action);
	virtual ~WhileStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
	
	const IExpression* Condition;
	const IStatement* Action;
};

struct BreakStatement : IStatement
{
	BreakStatement(const RSToken& token);
	virtual ~BreakStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
};

struct ContinueStatement : IStatement
{
	ContinueStatement(const RSToken& token);
	virtual ~ContinueStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
};

struct ForStatement : IStatement
{
	ForStatement(const RSToken& token, const IStatement* init, const IExpression* condition, const IStatement* post, const IStatement* action);
	virtual ~ForStatement() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
	
	const IStatement* Init;
	const IExpression* Condition;
	const IStatement* Post;
	const IStatement* Action;
};

struct StringLiteral : IExpression
{
	StringLiteral(const RSToken& token, const std::string& value);
	virtual ~StringLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
	
	std::string Value;
};

struct DecimalLiteral : IExpression
{
	DecimalLiteral(const RSToken& token, const float value);
	virtual ~DecimalLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
	
	float Value;
};

struct ArrayLiteral : IExpression
{
	ArrayLiteral(const RSToken& token, const std::vector<IExpression*>& elements);
	virtual ~ArrayLiteral() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
	
	std::vector<IExpression*> Elements;
};

struct IndexExpression : IExpression
{
	IndexExpression(const RSToken& token, const IExpression* left, const IExpression* index);
	virtual ~IndexExpression() = default;
	std::string ToString() const override;

	void Eval(Evaluator* evaluator) const;
	void Compile(Compiler* compiler) const;
	
	const IExpression* Left;
	const IExpression* Index;
};