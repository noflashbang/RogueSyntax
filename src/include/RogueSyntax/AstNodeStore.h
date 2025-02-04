#pragma once

#include "StandardLib.h"
#include "Identifiable.h"
#include "AstNode.h"

class AstNodeStore
{
public:
	AstNodeStore();
	~AstNodeStore();

	Identifier* New_Identifier(const RSToken& token, const std::string& value);
	LetStatement* New_LetStatement(const RSToken& token, const IExpression* name, const IExpression* value);
	ReturnStatement* New_ReturnStatement(const RSToken& token, const IExpression* returnValue);
	ExpressionStatement* New_ExpressionStatement(const RSToken& token, const IExpression* expression);
	NullLiteral* New_NullLiteral(const RSToken& token);
	IntegerLiteral* New_IntegerLiteral(const RSToken& token, const int value);
	BooleanLiteral* New_BooleanLiteral(const RSToken& token, const bool value);
	HashLiteral* New_HashLiteral(const RSToken& token, const std::map<IExpression*, IExpression*>& pairs);
	InfixExpression* New_InfixExpression(const RSToken& token, const IExpression* left, const std::string& op, const IExpression* right);
	PrefixExpression* New_PrefixExpression(const RSToken& token, const std::string& op, const IExpression* right);
	BlockStatement* New_BlockStatement(const RSToken& token, const std::vector<IStatement*>& statements);
	IfStatement* New_IfStatement(const RSToken& token, const IExpression* condition, const IStatement* consequence, const IStatement* alternative);
	FunctionLiteral* New_FunctionLiteral(const RSToken& token, const std::vector<IExpression*>& parameters, const IStatement* body);
	CallExpression* New_CallExpression(const RSToken& token, const IExpression* function, const std::vector<IExpression*>& arguments);
	WhileStatement* New_WhileStatement(const RSToken& token, const IExpression* condition, const IStatement* action);
	BreakStatement* New_BreakStatement(const RSToken& token);
	ContinueStatement* New_ContinueStatement(const RSToken& token);
	ForStatement* New_ForStatement(const RSToken& token, const IStatement* init, const IExpression* condition, const IStatement* post, const IStatement* action);
	StringLiteral* New_StringLiteral(const RSToken& token, const std::string& value);
	DecimalLiteral* New_DecimalLiteral(const RSToken& token, const float value);
	ArrayLiteral* New_ArrayLiteral(const RSToken& token, const std::vector<IExpression*>& elements);
	IndexExpression* New_IndexExpression(const RSToken& token, const IExpression* left, const IExpression* index);

private:
	std::vector<std::shared_ptr<INode>> _store;
};

