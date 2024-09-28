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
	EQUALS,      // ==
	LESSGREATER, // > or <
	SUM,         // +
	PRODUCT,     // *
	PREFIX,      // -X or !X
	CALL,        // myFunction(X)
	ASSIGN,      // =
	INDEX,       // array[index]
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
	std::shared_ptr<IStatement> ParseStatement();

	std::shared_ptr<IExpression> ParseExpression(const Precedence precedence);

	std::shared_ptr<IExpression> ParseIdentifier();
	std::shared_ptr<IExpression> ParseNullLiteral();
	std::shared_ptr<IExpression> ParseIntegerLiteral();
	std::shared_ptr<IExpression> ParseDecimalLiteral();
	std::shared_ptr<IExpression> ParseStringLiteral();
	std::shared_ptr<IExpression> ParseArrayLiteral();
	std::shared_ptr<IExpression> ParseHashLiteral();

	std::shared_ptr<IExpression> ParsePrefixExpression();
	std::shared_ptr<IExpression> ParseInfixExpression(const std::shared_ptr<IExpression>& left);
	std::shared_ptr<IExpression> ParseBoolean();
	std::shared_ptr<IExpression> ParseGroupedExpression();
	std::shared_ptr<IExpression> ParseFunctionLiteral();
	std::shared_ptr<IExpression> ParseCallExpression(const std::shared_ptr<IExpression>& function);
	std::shared_ptr<IExpression> ParseIndexExpression(const std::shared_ptr<IExpression>& left);
	std::shared_ptr<IExpression> ParseAssignExpression(const std::shared_ptr<IExpression>& left);
	std::shared_ptr<IExpression> ParseOpAssignExpression(const std::shared_ptr<IExpression>& left);

	std::shared_ptr<IStatement> ParseIfStatement();
	std::shared_ptr<IStatement> ParseWhileStatement();
	std::shared_ptr<IStatement> ParseForStatement();

	std::shared_ptr<IStatement> ParseBlockStatement();
	std::shared_ptr<IStatement> ParseLetStatement();
	std::shared_ptr<IStatement> ParseAssignStatement();
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
};