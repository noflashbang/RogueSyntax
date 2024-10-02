#pragma once

#include "StandardLib.h"
#include "Token.h"
#include "Lexer.h"
#include "AstNode.h"
#include "AstNodeStore.h"

typedef std::function<IExpression*()> PrefixParseFn;
typedef std::function<IExpression*(IExpression*)> InfixParseFn;

enum class Precedence
{
	LOWEST,
	EQUALS,      // ==
	LESSGREATER, // > or <
	SUM,         // +
	PRODUCT,     // *
	PREFIX,      // -X or !X
	CALL,        // myFunction(X)
	INDEX,       // array[index]
	ASSIGN,      // =
};

static const std::map<TokenType, Precedence> PRECEDENCES = {
	{TokenType::TOKEN_ASSIGN, Precedence::EQUALS},
	{TokenType::TOKEN_INCREMENT, Precedence::SUM},
	{TokenType::TOKEN_DECREMENT, Precedence::SUM},
	{TokenType::TOKEN_EQ, Precedence::EQUALS},
	{TokenType::TOKEN_NOT_EQ, Precedence::EQUALS},
	{TokenType::TOKEN_LT, Precedence::LESSGREATER},
	{TokenType::TOKEN_GT, Precedence::LESSGREATER},
	{TokenType::TOKEN_PLUS, Precedence::SUM},
	{TokenType::TOKEN_MINUS, Precedence::SUM},
	{TokenType::TOKEN_SLASH, Precedence::PRODUCT},
	{TokenType::TOKEN_ASTERISK, Precedence::PRODUCT},
	{TokenType::TOKEN_LPAREN, Precedence::CALL},
	{TokenType::TOKEN_LBRACKET, Precedence::INDEX},
	{TokenType::TOKEN_LT_EQ, Precedence::LESSGREATER},
	{TokenType::TOKEN_GT_EQ, Precedence::LESSGREATER},
	{TokenType::TOKEN_AND, Precedence::LESSGREATER},
	{TokenType::TOKEN_OR, Precedence::LESSGREATER},
	{TokenType::TOKEN_BITWISE_AND, Precedence::LESSGREATER},
	{TokenType::TOKEN_BITWISE_OR, Precedence::LESSGREATER},
	{TokenType::TOKEN_BITWISE_XOR, Precedence::LESSGREATER},
	{TokenType::TOKEN_SHIFT_LEFT, Precedence::LESSGREATER},
	{TokenType::TOKEN_SHIFT_RIGHT, Precedence::LESSGREATER},
	{TokenType::TOKEN_MODULO, Precedence::PRODUCT},
	{TokenType::TOKEN_MODULO_ASSIGN, Precedence::PRODUCT},
	{TokenType::TOKEN_PLUS_ASSIGN, Precedence::ASSIGN},
	{TokenType::TOKEN_MINUS_ASSIGN, Precedence::ASSIGN},
	{TokenType::TOKEN_ASTERISK_ASSIGN, Precedence::ASSIGN},
	{TokenType::TOKEN_SLASH_ASSIGN, Precedence::ASSIGN},
};

class Parser
{
public:
	Parser(const Lexer& lexer);

	void NextToken();
	std::shared_ptr<Program> ParseProgram();
	
	IStatement* ParseStatement();

	IExpression* ParseExpression(const Precedence precedence);

	IExpression* ParseIdentifier();
	IExpression* ParseNullLiteral();
	IExpression* ParseIntegerLiteral();
	IExpression* ParseDecimalLiteral();
	IExpression* ParseStringLiteral();
	IExpression* ParseArrayLiteral();
	IExpression* ParseHashLiteral();

	IExpression* ParsePrefixExpression();
	IExpression* ParseInfixExpression(const IExpression* left);
	IExpression* ParseBoolean();
	IExpression* ParseGroupedExpression();
	IExpression* ParseFunctionLiteral();
	IExpression* ParseCallExpression(const IExpression* function);
	IExpression* ParseIndexExpression(const IExpression* left);
	IExpression* ParseAssignExpression(const IExpression* left);
	IExpression* ParseOpAssignExpression(const IExpression* left);

	IStatement* ParseIfStatement();
	IStatement* ParseWhileStatement();
	IStatement* ParseForStatement();

	IStatement* ParseBlockStatement();
	IStatement* ParseLetStatement();
	IStatement* ParseAssignStatement();
	IStatement* ParseReturnStatement();
	IStatement* ParseBreakStatement();
	IStatement* ParseContinueStatement();
	IStatement* ParseExpressionStatement();

	bool ExpectPeek(const TokenType& expectedType);
	bool CurrentTokenIs(const TokenType& type) const;
	bool PeekTokenIs(const TokenType& type) const;

	std::vector<std::string> Errors() const;
	void AddError(const std::string& error);

	void PeekError(const TokenType& type);

private:

	void RegisterPrefix(const TokenType& type, const PrefixParseFn& fn);
	void RegisterInfix(const TokenType& type, const InfixParseFn& fn);

	Precedence PeekPrecedence() const;
	Precedence CurrentPrecedence() const;

	Lexer _lexer;
	Token _currentToken;
	Token _nextToken;

	std::vector<std::string> _errors;
	std::map<TokenType, PrefixParseFn> _prefixDispatch;
	std::map<TokenType, InfixParseFn> _infixDispatch;

	std::shared_ptr<AstNodeStore> _currentStore;
};