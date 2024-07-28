#pragma once

#include "StandardLib.h"
#include "Token.h"
#include "Lexer.h"
#include "AstNode.h"

typedef std::function<std::unique_ptr<IExpression>()> PrefixParseFn;
typedef std::function<std::unique_ptr<IExpression>(std::unique_ptr<IExpression>)> InfixParseFn;

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
	std::unique_ptr<IStatement> ParseStatement();

	std::unique_ptr<IExpression> ParseExpression(const Precedence precedence);

	std::unique_ptr<IExpression> ParseIdentifier();
	std::unique_ptr<IExpression> ParseIntegerLiteral();
	std::unique_ptr<IExpression> ParsePrefixExpression();
	std::unique_ptr<IExpression> ParseInfixExpression(std::unique_ptr<IExpression> left);
	std::unique_ptr<IExpression> ParseBoolean();
	std::unique_ptr<IExpression> ParseGroupedExpression();
	std::unique_ptr<IExpression> ParseIfExpression();
	std::unique_ptr<IExpression> ParseFunctionLiteral();
	std::unique_ptr<IExpression> ParseCallExpression(std::unique_ptr<IExpression> function);

	std::unique_ptr<IStatement> ParseBlockStatement();
	std::unique_ptr<IStatement> ParseLetStatement();
	std::unique_ptr<IStatement> ParseReturnStatement();
	std::unique_ptr<IStatement> ParseExpressionStatement();


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