#pragma once

#include "StandardLib.h"
#include "Token.h"
#include "Lexer.h"
#include "AstNode.h"

typedef std::function<std::shared_ptr<IExpression>()> PrefixParseFn;
typedef std::function<std::shared_ptr<IExpression>(std::shared_ptr<IExpression>)> InfixParseFn;

enum class Precedence
{
	LOWEST,
	//ASSIGN,      // =
	EQUALS,      // ==
	LESSGREATER, // > or <
	SUM,         // +
	PRODUCT,     // *
	PREFIX,      // -X or !X
	CALL,        // myFunction(X)
};

static const std::map<TokenType, Precedence> PRECEDENCES = {
	//{TokenType::TOKEN_ASSIGN, Precedence::EQUALS},
	//{TokenType::TOKEN_INCREMENT, Precedence::EQUALS},
	//{TokenType::TOKEN_DECREMENT, Precedence::EQUALS},
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
	std::shared_ptr<Program> ParseProgram();
	std::shared_ptr<IStatement> ParseStatement();

	std::shared_ptr<IExpression> ParseExpression(const Precedence precedence);

	std::shared_ptr<IExpression> ParseIdentifier();
	std::shared_ptr<IExpression> ParseIntegerLiteral();
	std::shared_ptr<IExpression> ParsePrefixExpression();
	std::shared_ptr<IExpression> ParseInfixExpression(std::shared_ptr<IExpression> left);
	std::shared_ptr<IExpression> ParseBoolean();
	std::shared_ptr<IExpression> ParseGroupedExpression();
	std::shared_ptr<IExpression> ParseIfExpression();
	std::shared_ptr<IExpression> ParseFunctionLiteral();
	std::shared_ptr<IExpression> ParseCallExpression(std::shared_ptr<IExpression> function);
	std::shared_ptr<IExpression> ParseWhileExpression();
	std::shared_ptr<IExpression> ParseForExpression();

	std::shared_ptr<IStatement> ParseBlockStatement();
	std::shared_ptr<IStatement> ParseLetStatement();
	std::shared_ptr<IStatement> ParseAssignStatement();
	std::shared_ptr<IStatement> ParseIncrementStatement();
	std::shared_ptr<IStatement> ParseDecrementStatement();
	std::shared_ptr<IStatement> ParseReturnStatement();
	std::shared_ptr<IStatement> ParseBreakStatement();
	std::shared_ptr<IStatement> ParseContinueStatement();
	std::shared_ptr<IStatement> ParseExpressionStatement();


	bool ExpectPeek(const TokenType& expectedType);
	bool CurrentTokenIs(const TokenType& type) const;
	bool PeekTokenIs(const TokenType& type) const;

	std::vector<std::string> Errors() const;
	void AddError(const std::string& error);

	void PeekError(const TokenType& type);

private:

	void RegisterPrefix(const TokenType& type, PrefixParseFn fn);
	void RegisterInfix(const TokenType& type, InfixParseFn fn);

	Precedence PeekPrecedence() const;
	Precedence CurrentPrecedence() const;

	Lexer _lexer;
	Token _currentToken;
	Token _nextToken;

	std::vector<std::string> _errors;
	std::map<TokenType, PrefixParseFn> _prefixDispatch;
	std::map<TokenType, InfixParseFn> _infixDispatch;
};