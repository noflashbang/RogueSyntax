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
		IStatement* pStatement = ParseStatement();
		if (pStatement != nullptr)
		{
			program.Statements.push_back(pStatement);
		}
		
		NextToken();
	}
	return program;
}

IStatement* Parser::ParseStatement()
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

IExpression* Parser::ParseExpression(const Precedence precedence)
{
	auto prefix = _prefixDispatch.find(_currentToken.Type);
	if (prefix == _prefixDispatch.end())
	{
		std::string error = "no prefix parse function for " + _currentToken.Type.Name + " found";
		AddError(error);
		return nullptr;
	}

	IExpression* left = prefix->second();

	while (!PeekTokenIs(TokenType::TOKEN_SEMICOLON) && precedence < PeekPrecedence())
	{
		auto infix = _infixDispatch.find(_nextToken.Type);
		if (infix == _infixDispatch.end())
		{
			return left;
		}
	
		NextToken();
	
		left = infix->second(left);
	}
	return left;
}

IExpression* Parser::ParseIdentifier()
{
	Identifier* pIdentifier = new Identifier();
	pIdentifier->Token = _currentToken;
	pIdentifier->Value = _currentToken.Literal;
	return pIdentifier;
}

IExpression* Parser::ParseIntegerLiteral()
{
	IntegerLiteral* pIntegerLiteral = new IntegerLiteral();
	pIntegerLiteral->Token = _currentToken;

	try
	{
		pIntegerLiteral->Value = std::stoi(_currentToken.Literal);
	}
	catch (const std::invalid_argument& e)
	{
		std::string error = "could not parse " + _currentToken.Literal + " as integer";
		AddError(error);
		return nullptr;
	}
	
	return pIntegerLiteral;
}

IExpression* Parser::ParsePrefixExpression()
{
	PrefixExpression* pPrefixExpression = new PrefixExpression();
	pPrefixExpression->Token = _currentToken;
	pPrefixExpression->Operator = _currentToken.Literal;

	NextToken();

	pPrefixExpression->Right = ParseExpression(Precedence::PREFIX);

	return pPrefixExpression;
}

IExpression* Parser::ParseInfixExpression(IExpression* left)
{
	InfixExpression* pInfixExpression = new InfixExpression();
	pInfixExpression->Token = _currentToken;
	pInfixExpression->Operator = _currentToken.Literal;
	pInfixExpression->Left = left;

	Precedence precedence = CurrentPrecedence();
	NextToken();
	pInfixExpression->Right = ParseExpression(precedence);

	return pInfixExpression;
}

IStatement* Parser::ParseLetStatement()
{
	LetStatement* pLetStatement = new LetStatement();
	pLetStatement->Token = _currentToken;

	if (!ExpectPeek(TokenType::TOKEN_IDENT))
	{
		return nullptr;
	}

	pLetStatement->Name = new Identifier();
	pLetStatement->Name->Token = _currentToken;
	pLetStatement->Name->Value = _currentToken.Literal;

	if (!ExpectPeek(TokenType::TOKEN_ASSIGN))
	{
		return nullptr;
	}

	NextToken();
	pLetStatement->Value = ParseExpression(Precedence::LOWEST);

	while(_currentToken.Type != TokenType::TOKEN_SEMICOLON)
	{
		NextToken();
	}

	return pLetStatement;
}

IStatement* Parser::ParseReturnStatement()
{
	ReturnStatement* pReturnStatement = new ReturnStatement();
	pReturnStatement->Token = _currentToken;

	NextToken();

	pReturnStatement->ReturnValue = ParseExpression(Precedence::LOWEST);

	while (_currentToken.Type != TokenType::TOKEN_SEMICOLON)
	{
		NextToken();
	}

	return pReturnStatement;
}

IStatement* Parser::ParseExpressionStatement()
{
	ExpressionStatement* pExpressionStatement = new ExpressionStatement();
	pExpressionStatement->Token = _currentToken;

	//parse the expression
	pExpressionStatement->Expression = ParseExpression(Precedence::LOWEST);

	//if the next token is a semicolon, consume it
	if (PeekTokenIs(TokenType::TOKEN_SEMICOLON))
	{
		NextToken();
	}

	return pExpressionStatement;
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