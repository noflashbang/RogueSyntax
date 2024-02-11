#pragma once

#include "StandardLib.h"
#include "Token.h"
#include "Lexer.h"
#include "AstNode.h"

typedef std::function<IExpression* ()> PrefixParseFn;
typedef std::function<IExpression* (IExpression*)> InfixParseFn;

enum class Precedence
{
	LOWEST,
	EQUALS,      // ==
	LESSGREATER, // > or <
	SUM,         // +
	PRODUCT,     // *
	PREFIX,      // -X or !X
	CALL,        // myFunction(X)
};

static const std::map<TokenType, Precedence> PRECEDENCES = {
	{TokenType::TOKEN_EQ, Precedence::EQUALS},
	{TokenType::TOKEN_NOT_EQ, Precedence::EQUALS},
	{TokenType::TOKEN_LT, Precedence::LESSGREATER},
	{TokenType::TOKEN_GT, Precedence::LESSGREATER},
	{TokenType::TOKEN_PLUS, Precedence::SUM},
	{TokenType::TOKEN_MINUS, Precedence::SUM},
	{TokenType::TOKEN_SLASH, Precedence::PRODUCT},
	{TokenType::TOKEN_ASTERISK, Precedence::PRODUCT},
	{TokenType::TOKEN_LPAREN, Precedence::CALL},
};

class Parser
{
public:
	Parser(Lexer lexer);

	void NextToken();
	Program ParseProgram();
	IStatement* ParseStatement();

	IExpression* ParseExpression(const Precedence precedence);

	IExpression* ParseIdentifier();
	IExpression* ParseIntegerLiteral();
	IExpression* ParsePrefixExpression();
	IExpression* ParseInfixExpression(IExpression* left);

	IStatement* ParseLetStatement();
	IStatement* ParseReturnStatement();
	IStatement* ParseExpressionStatement();


	bool ExpectPeek(const TokenType expectedType);
	bool CurrentTokenIs(const TokenType type) const;
	bool PeekTokenIs(const TokenType type) const;

	std::vector<std::string> Errors() const;
	void AddError(const std::string& error);

	void PeekError(const TokenType type);

private:

	void RegisterPrefix(TokenType type, PrefixParseFn fn);
	void RegisterInfix(TokenType type, InfixParseFn fn);

	Precedence PeekPrecedence() const;
	Precedence CurrentPrecedence() const;

	Lexer _lexer;
	Token _currentToken;
	Token _nextToken;

	std::vector<std::string> _errors;
	

	std::map<TokenType, PrefixParseFn> _prefixDispatch;
	std::map<TokenType, InfixParseFn> _infixDispatch;
};