#include "pch.h"

AstNodeStore::AstNodeStore()
{
	_store.reserve(1024);
}

AstNodeStore::~AstNodeStore()
{
}

Identifier* AstNodeStore::New_Identifier(const RSToken& token, const std::string& value)
{
	auto node = std::make_shared<Identifier>(token, value);
	_store.push_back(node);
	return node.get();
}

LetStatement* AstNodeStore::New_LetStatement(const RSToken& token, const IExpression* name, const IExpression* value)
{
	auto node = std::make_shared<LetStatement>(token, name, value);
	_store.push_back(node);
	return node.get();
}

ReturnStatement* AstNodeStore::New_ReturnStatement(const RSToken& token, const IExpression* returnValue)
{
	auto node = std::make_shared<ReturnStatement>(token, returnValue);
	_store.push_back(node);
	return node.get();
}

ExpressionStatement* AstNodeStore::New_ExpressionStatement(const RSToken& token, const IExpression* expression)
{
	auto node = std::make_shared<ExpressionStatement>(token, expression);
	_store.push_back(node);
	return node.get();
}

NullLiteral* AstNodeStore::New_NullLiteral(const RSToken& token)
{
	auto node = std::make_shared<NullLiteral>(token);
	_store.push_back(node);
	return node.get();
}

IntegerLiteral* AstNodeStore::New_IntegerLiteral(const RSToken& token, const int value)
{
	auto node = std::make_shared<IntegerLiteral>(token, value);
	_store.push_back(node);
	return node.get();
}

BooleanLiteral* AstNodeStore::New_BooleanLiteral(const RSToken& token, const bool value)
{
	auto node = std::make_shared<BooleanLiteral>(token, value);
	_store.push_back(node);
	return node.get();
}

HashLiteral* AstNodeStore::New_HashLiteral(const RSToken& token, const std::map<IExpression*, IExpression*>& pairs)
{
	auto node = std::make_shared<HashLiteral>(token, pairs);
	_store.push_back(node);
	return node.get();
}

InfixExpression* AstNodeStore::New_InfixExpression(const RSToken& token, const IExpression* left, const std::string& op, const IExpression* right)
{
	auto node = std::make_shared<InfixExpression>(token, left, op, right);
	_store.push_back(node);
	return node.get();
}

PrefixExpression* AstNodeStore::New_PrefixExpression(const RSToken& token, const std::string& op, const IExpression* right)
{
	auto node = std::make_shared<PrefixExpression>(token, op, right);
	_store.push_back(node);
	return node.get();
}

BlockStatement* AstNodeStore::New_BlockStatement(const RSToken& token, const std::vector<IStatement*>& statements)
{
	auto node = std::make_shared<BlockStatement>(token, statements);
	_store.push_back(node);
	return node.get();
}

IfStatement* AstNodeStore::New_IfStatement(const RSToken& token, const IExpression* condition, const IStatement* consequence, const IStatement* alternative)
{
	auto node = std::make_shared<IfStatement>(token, condition, consequence, alternative);
	_store.push_back(node);
	return node.get();
}

FunctionLiteral* AstNodeStore::New_FunctionLiteral(const RSToken& token, const std::vector<IExpression*>& parameters, const IStatement* body)
{
	auto node = std::make_shared<FunctionLiteral>(token, parameters, body);
	_store.push_back(node);
	return node.get();
}

CallExpression* AstNodeStore::New_CallExpression(const RSToken& token, const IExpression* function, const std::vector<IExpression*>& arguments)
{
	auto node = std::make_shared<CallExpression>(token, function, arguments);
	_store.push_back(node);
	return node.get();
}

WhileStatement* AstNodeStore::New_WhileStatement(const RSToken& token, const IExpression* condition, const IStatement* action)
{
	auto node = std::make_shared<WhileStatement>(token, condition, action);
	_store.push_back(node);
	return node.get();
}

BreakStatement* AstNodeStore::New_BreakStatement(const RSToken& token)
{
	auto node = std::make_shared<BreakStatement>(token);
	_store.push_back(node);
	return node.get();
}

ContinueStatement* AstNodeStore::New_ContinueStatement(const RSToken& token)
{
	auto node = std::make_shared<ContinueStatement>(token);
	_store.push_back(node);
	return node.get();
}

ForStatement* AstNodeStore::New_ForStatement(const RSToken& token, const IStatement* init, const IExpression* condition, const IStatement* post, const IStatement* action)
{
	auto node = std::make_shared<ForStatement>(token, init, condition, post, action);
	_store.push_back(node);
	return node.get();
}

StringLiteral* AstNodeStore::New_StringLiteral(const RSToken& token, const std::string& value)
{
	auto node = std::make_shared<StringLiteral>(token, value);
	_store.push_back(node);
	return node.get();
}

DecimalLiteral* AstNodeStore::New_DecimalLiteral(const RSToken& token, const float value)
{
	auto node = std::make_shared<DecimalLiteral>(token, value);
	_store.push_back(node);
	return node.get();
}

ArrayLiteral* AstNodeStore::New_ArrayLiteral(const RSToken& token, const std::vector<IExpression*>& elements)
{
	auto node = std::make_shared<ArrayLiteral>(token, elements);
	_store.push_back(node);
	return node.get();
}

IndexExpression* AstNodeStore::New_IndexExpression(const RSToken& token, const IExpression* left, const IExpression* index)
{
	auto node = std::make_shared<IndexExpression>(token, left, index);
	_store.push_back(node);
	return node.get();
}
