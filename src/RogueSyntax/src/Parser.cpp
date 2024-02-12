#include "Parser.h"

Parser::Parser(Lexer lexer) : _lexer(lexer)
{
	_prefixDispatch[TokenType::TOKEN_IDENT] = std::bind(&Parser::ParseIdentifier, this);
	_prefixDispatch[TokenType::TOKEN_INT] = std::bind(&Parser::ParseIntegerLiteral, this);
	_prefixDispatch[TokenType::TOKEN_BANG] = std::bind(&Parser::ParsePrefixExpression, this);
	_prefixDispatch[TokenType::TOKEN_MINUS] = std::bind(&Parser::ParsePrefixExpression, this);

	//register infix operators
	_infixDispatch[TokenType::TOKEN_PLUS] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_MINUS] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_SLASH] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_ASTERISK] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_EQ] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_NOT_EQ] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_LT] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_GT] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);

	//load the first two tokens
	NextToken();
	NextToken();
}

void Parser::NextToken()
{
	_currentToken = _nextToken;
	_nextToken = _lexer.NextToken();
}

Program Parser::ParseProgram()
{
	Program program;

	while (_currentToken.Type != TokenType::TOKEN_EOF)
	{
		auto pStatement = ParseStatement();
		if (pStatement != nullptr)
		{
			program.Statements.push_back(std::move(pStatement));
		}
		NextToken();
	}
	return program;
}

std::unique_ptr<IStatement> Parser::ParseStatement()
{
	if (_currentToken.Type == TokenType::TOKEN_LET)
	{
		return ParseLetStatement();
	}
	else if(_currentToken.Type == TokenType::TOKEN_RETURN)
	{
		return ParseReturnStatement();
	}
	else
	{
		return ParseExpressionStatement();
	}
}

std::unique_ptr<IExpression> Parser::ParseExpression(const Precedence precedence)
{
	auto prefix = _prefixDispatch.find(_currentToken.Type);
	if (prefix == _prefixDispatch.end())
	{
		std::string error = "no prefix parse function for " + _currentToken.Type.Name + " found";
		AddError(error);
		return nullptr;
	}

	auto left = prefix->second();

	while (!PeekTokenIs(TokenType::TOKEN_SEMICOLON) && precedence < PeekPrecedence())
	{
		auto infix = _infixDispatch.find(_nextToken.Type);
		if (infix == _infixDispatch.end())
		{
			return left;
		}
	
		NextToken();
	
		left = infix->second(std::move(left));
	}
	return left;
}

std::unique_ptr<IExpression> Parser::ParseIdentifier()
{
	return Identifier::New(_currentToken, _currentToken.Literal);
}

std::unique_ptr<IExpression> Parser::ParseIntegerLiteral()
{
	try
	{
		return IntegerLiteral::New(_currentToken, std::stoi(_currentToken.Literal));
	}
	catch (const std::invalid_argument& e)
	{
		std::string error = "could not parse " + _currentToken.Literal + " as integer";
		AddError(error);
		return nullptr;
	}
}

std::unique_ptr<IExpression> Parser::ParsePrefixExpression()
{
	auto token = _currentToken;
	auto op = _currentToken.Literal;

	NextToken();

	auto right = ParseExpression(Precedence::PREFIX);

	return PrefixExpression::New(token, op, std::move(right));
}

std::unique_ptr<IExpression> Parser::ParseInfixExpression(std::unique_ptr<IExpression> left)
{	
	auto token = _currentToken;
	auto op = _currentToken.Literal;

	Precedence precedence = CurrentPrecedence();
	NextToken();
	auto right = ParseExpression(precedence);

	return InfixExpression::New(token, std::move(left), op, std::move(right));
}

std::unique_ptr<IStatement> Parser::ParseLetStatement()
{
	auto token = _currentToken;

	if (!ExpectPeek(TokenType::TOKEN_IDENT))
	{
		return nullptr;
	}

	auto identifier = Identifier::New(_currentToken, _currentToken.Literal);

	if (!ExpectPeek(TokenType::TOKEN_ASSIGN))
	{
		return nullptr;
	}

	NextToken();
	
	auto value = ParseExpression(Precedence::LOWEST);

	while(_currentToken.Type != TokenType::TOKEN_SEMICOLON)
	{
		NextToken();
	}

	return LetStatement::New(token, std::move(identifier), std::move(value));
}

std::unique_ptr<IStatement> Parser::ParseReturnStatement()
{
	auto token = _currentToken;

	NextToken();

	auto returnValue = ParseExpression(Precedence::LOWEST);

	while (_currentToken.Type != TokenType::TOKEN_SEMICOLON)
	{
		NextToken();
	}

	return ReturnStatement::New(token, std::move(returnValue));
}

std::unique_ptr<IStatement> Parser::ParseExpressionStatement()
{
	auto token = _currentToken;

	//parse the expression
	auto expression = ParseExpression(Precedence::LOWEST);

	//if the next token is a semicolon, consume it
	if (PeekTokenIs(TokenType::TOKEN_SEMICOLON))
	{
		NextToken();
	}

	return ExpressionStatement::New(token, std::move(expression));
}


bool Parser::ExpectPeek(const TokenType expectedType)
{
	if (_nextToken.Type == expectedType)
	{
		NextToken();
		return true;
	}
	else
	{
		PeekError(expectedType);
		return false;
	}
}

bool Parser::PeekTokenIs(const TokenType expectedType) const
{
	return _nextToken.Type == expectedType;
}

bool Parser::CurrentTokenIs(const TokenType expectedType) const
{
	return _currentToken.Type == expectedType;
}

std::vector<std::string> Parser::Errors() const
{
	return _errors;
}

void Parser::AddError(const std::string& error)
{
	_errors.push_back(error);
}

void Parser::PeekError(const TokenType expectedType)
{
	std::string error = "expected next token to be " + expectedType.Name + ", got " + _nextToken.Type.Name + " instead";
	AddError(error);
}

void Parser::RegisterInfix(TokenType type, InfixParseFn fn)
{
	_infixDispatch[type] = fn;
}

void Parser::RegisterPrefix(TokenType type, PrefixParseFn fn)
{
	_prefixDispatch[type] = fn;
}

Precedence Parser::CurrentPrecedence() const
{
	auto it = PRECEDENCES.find(_currentToken.Type);
	if (it != PRECEDENCES.end())
	{
		return it->second;
	}
	return Precedence::LOWEST;
}

Precedence Parser::PeekPrecedence() const
{
	auto it = PRECEDENCES.find(_nextToken.Type);
	if (it != PRECEDENCES.end())
	{
		return it->second;
	}
	return Precedence::LOWEST;
}