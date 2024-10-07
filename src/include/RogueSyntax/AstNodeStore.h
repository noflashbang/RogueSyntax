#pragma once

#include "StandardLib.h"
#include "Identifiable.h"
#include "AstNode.h"

class AstNodeStore
{
public:
	AstNodeStore();
	~AstNodeStore();

	Identifier* New_Identifier(const Token& token, const std::string& value);
	LetStatement* New_LetStatement(const Token& token, const IExpression* name, const IExpression* value);
	ReturnStatement* New_ReturnStatement(const Token& token, const IExpression* returnValue);
	ExpressionStatement* New_ExpressionStatement(const Token& token, const IExpression* expression);
	NullLiteral* New_NullLiteral(const Token& token);
	IntegerLiteral* New_IntegerLiteral(const Token& token, const int value);
	BooleanLiteral* New_BooleanLiteral(const Token& token, const bool value);
	HashLiteral* New_HashLiteral(const Token& token, const std::map<IExpression*, IExpression*>& pairs);
	InfixExpression* New_InfixExpression(const Token& token, const IExpression* left, const std::string& op, const IExpression* right);
	PrefixExpression* New_PrefixExpression(const Token& token, const std::string& op, const IExpression* right);
	BlockStatement* New_BlockStatement(const Token& token, const std::vector<IStatement*>& statements);
	IfStatement* New_IfStatement(const Token& token, const IExpression* condition, const IStatement* consequence, const IStatement* alternative);
	FunctionLiteral* New_FunctionLiteral(const Token& token, const std::vector<IExpression*>& parameters, const IStatement* body);
	CallExpression* New_CallExpression(const Token& token, const IExpression* function, const std::vector<IExpression*>& arguments);
	WhileStatement* New_WhileStatement(const Token& token, const IExpression* condition, const IStatement* action);
	BreakStatement* New_BreakStatement(const Token& token);
	ContinueStatement* New_ContinueStatement(const Token& token);
	ForStatement* New_ForStatement(const Token& token, const IStatement* init, const IExpression* condition, const IStatement* post, const IStatement* action);
	StringLiteral* New_StringLiteral(const Token& token, const std::string& value);
	DecimalLiteral* New_DecimalLiteral(const Token& token, const float value);
	ArrayLiteral* New_ArrayLiteral(const Token& token, const std::vector<IExpression*>& elements);
	IndexExpression* New_IndexExpression(const Token& token, const IExpression* left, const IExpression* index);

private:
	std::vector<std::shared_ptr<INode>> _store;
};

