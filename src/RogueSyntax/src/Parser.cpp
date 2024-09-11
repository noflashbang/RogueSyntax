#include "Parser.h"
#include "Parser.h"
#include "Parser.h"
#include "Parser.h"
#include "Parser.h"
#include "Parser.h"
#include "Parser.h"
#include "pch.h"

Parser::Parser(const Lexer& lexer) : _lexer(lexer)
{
	_prefixDispatch[TokenType::TOKEN_IDENT] = std::bind(&Parser::ParseIdentifier, this);
	_prefixDispatch[TokenType::TOKEN_INT] = std::bind(&Parser::ParseIntegerLiteral, this);
	_prefixDispatch[TokenType::TOKEN_NULL] = std::bind(&Parser::ParseNullLiteral, this);
	_prefixDispatch[TokenType::TOKEN_DECIMAL] = std::bind(&Parser::ParseDecimalLiteral, this);
	_prefixDispatch[TokenType::TOKEN_STRING] = std::bind(&Parser::ParseStringLiteral, this);
	_prefixDispatch[TokenType::TOKEN_BANG] = std::bind(&Parser::ParsePrefixExpression, this);
	_prefixDispatch[TokenType::TOKEN_MINUS] = std::bind(&Parser::ParsePrefixExpression, this);
	_prefixDispatch[TokenType::TOKEN_FALSE] = std::bind(&Parser::ParseBoolean, this);
	_prefixDispatch[TokenType::TOKEN_TRUE] = std::bind(&Parser::ParseBoolean, this);
	_prefixDispatch[TokenType::TOKEN_LPAREN] = std::bind(&Parser::ParseGroupedExpression, this);
	_prefixDispatch[TokenType::TOKEN_IF] = std::bind(&Parser::ParseIfExpression, this);
	_prefixDispatch[TokenType::TOKEN_FUNCTION] = std::bind(&Parser::ParseFunctionLiteral, this);
	_prefixDispatch[TokenType::TOKEN_WHILE] = std::bind(&Parser::ParseWhileExpression, this);
	_prefixDispatch[TokenType::TOKEN_FOR] = std::bind(&Parser::ParseForExpression, this);
	_prefixDispatch[TokenType::TOKEN_LBRACKET] = std::bind(&Parser::ParseArrayLiteral, this);
	_prefixDispatch[TokenType::TOKEN_LBRACE] = std::bind(&Parser::ParseHashLiteral, this);

	//register infix operators
	_infixDispatch[TokenType::TOKEN_PLUS] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_MINUS] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_SLASH] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_ASTERISK] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_EQ] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_NOT_EQ] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_LT] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_GT] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_LPAREN] = std::bind(&Parser::ParseCallExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_LBRACKET] = std::bind(&Parser::ParseIndexExpression, this, std::placeholders::_1);
	//_infixDispatch[TokenType::TOKEN_ASSIGN] = std::bind(&Parser::ParseAssignExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_INCREMENT] = std::bind(&Parser::ParseIncrementExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_DECREMENT] = std::bind(&Parser::ParseIncrementExpression, this, std::placeholders::_1);

	//load the first two tokens
	NextToken();
	NextToken();
}

void Parser::NextToken()
{
	_currentToken = _nextToken;
	_nextToken = _lexer.NextToken();
}

std::shared_ptr<Program> Parser::ParseProgram()
{
	auto program = Program::New();

	while (_currentToken.Type != TokenType::TOKEN_EOF)
	{
		auto pStatement = ParseStatement();
		if (pStatement != nullptr)
		{
			program->Statements.push_back(std::move(pStatement));
		}
		NextToken();
	}
	return program;
}

std::shared_ptr<IStatement> Parser::ParseStatement()
{
	std::shared_ptr<IStatement> statement = nullptr;
	if (_currentToken.Type == TokenType::TOKEN_LET)
	{
		statement = ParseLetStatement();
	}
	else if (_nextToken.Type == TokenType::TOKEN_ASSIGN)
	{
		statement = ParseAssignStatement();
	}
	//else if (_currentToken.Type == TokenType::TOKEN_IDENT && _nextToken.Type == TokenType::TOKEN_INCREMENT)
	//{
	//	statement = ParseIncrementStatement();
	//}
	//else if (_currentToken.Type == TokenType::TOKEN_IDENT && _nextToken.Type == TokenType::TOKEN_DECREMENT)
	//{
	//	statement = ParseDecrementStatement();
	//}
	else if(_currentToken.Type == TokenType::TOKEN_RETURN)
	{
		statement = ParseReturnStatement();
	}
	else if (_currentToken.Type == TokenType::TOKEN_BREAK)
	{
		statement = ParseBreakStatement();
	}
	else if (_currentToken.Type == TokenType::TOKEN_CONTINUE)
	{
		statement = ParseContinueStatement();
	}
	else
	{
		statement = ParseExpressionStatement();
	}
	return statement;
}

std::shared_ptr<IExpression> Parser::ParseExpression(const Precedence precedence)
{
	auto prefix = _prefixDispatch.find(_currentToken.Type);
	if (prefix == _prefixDispatch.end())
	{
		std::string error = "No prefix parse function for " + _currentToken.Type.Name + " found";
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

std::shared_ptr<IExpression> Parser::ParseIdentifier()
{
	return Identifier::New(_currentToken, _currentToken.Literal);
}

std::shared_ptr<IExpression> Parser::ParseNullLiteral()
{
	return NullLiteral::New(_currentToken);
}

std::shared_ptr<IExpression> Parser::ParseIntegerLiteral()
{
	try
	{
		return IntegerLiteral::New(_currentToken, std::stoi(_currentToken.Literal));
	}
	catch (const std::invalid_argument& e)
	{
		std::string error = "could not parse " + _currentToken.Literal + " as integer";
		AddError(error);
		AddError(e.what());
		return nullptr;
	}
}

std::shared_ptr<IExpression> Parser::ParseDecimalLiteral()
{
	try
	{
		//remove the trailing d
		if (_currentToken.Literal.ends_with('d'))
		{
			_currentToken.Literal.pop_back();
		}
		return DecimalLiteral::New(_currentToken, std::stof(_currentToken.Literal));
	}
	catch (const std::invalid_argument& e)
	{
		std::string error = "could not parse " + _currentToken.Literal + " as decimal";
		AddError(error);
		AddError(e.what());
		return nullptr;
	}
}

std::shared_ptr<IExpression> Parser::ParseStringLiteral()
{
	//remove the quotes
	std::string result(_currentToken.Literal.begin() + 1, _currentToken.Literal.end() - 1);

	return StringLiteral::New(_currentToken, result);
}

std::shared_ptr<IExpression> Parser::ParsePrefixExpression()
{
	auto token = _currentToken;
	auto op = _currentToken.Literal;

	NextToken();

	auto right = ParseExpression(Precedence::PREFIX);

	return PrefixExpression::New(token, op, right);
}

std::shared_ptr<IExpression> Parser::ParseInfixExpression(const std::shared_ptr<IExpression>& left)
{	
	auto token = _currentToken;
	auto op = _currentToken.Literal;

	Precedence precedence = CurrentPrecedence();
	NextToken();
	auto right = ParseExpression(precedence);

	return InfixExpression::New(token, left, op, right);
}

std::shared_ptr<IExpression> Parser::ParseBoolean()
{
	return BooleanLiteral::New(_currentToken, _currentToken.Type == TokenType::TOKEN_TRUE);
}

std::shared_ptr<IExpression> Parser::ParseGroupedExpression()
{
	NextToken();

	auto expression = ParseExpression(Precedence::LOWEST);

	if (!ExpectPeek(TokenType::TOKEN_RPAREN))
	{
		return nullptr;
	}

	return expression;
}

std::shared_ptr<IExpression> Parser::ParseIfExpression()
{
	auto token = _currentToken;

	if (!ExpectPeek(TokenType::TOKEN_LPAREN))
	{
		return nullptr;
	}

	NextToken();

	auto condition = ParseExpression(Precedence::LOWEST);

	if (!ExpectPeek(TokenType::TOKEN_RPAREN))
	{
		return nullptr;
	}

	if (!ExpectPeek(TokenType::TOKEN_LBRACE))
	{
		return nullptr;
	}

	//parse the consequence
	auto consequence = ParseBlockStatement();

	std::shared_ptr<IStatement> alternative = nullptr;

	if (PeekTokenIs(TokenType::TOKEN_ELSE))
	{
		NextToken();

		if (!ExpectPeek(TokenType::TOKEN_LBRACE))
		{
			return nullptr;
		}
		auto alt = ParseBlockStatement();
		alternative.swap(alt);
	}

	return IfExpression::New(token, condition, consequence, alternative);
}

std::shared_ptr<IExpression> Parser::ParseWhileExpression()
{
	auto token = _currentToken;

	if (!ExpectPeek(TokenType::TOKEN_LPAREN))
	{
		return nullptr;
	}

	NextToken();

	auto condition = ParseExpression(Precedence::LOWEST);

	if (!ExpectPeek(TokenType::TOKEN_RPAREN))
	{
		return nullptr;
	}

	if (!ExpectPeek(TokenType::TOKEN_LBRACE))
	{
		return nullptr;
	}

	//parse the consequence
	auto action = ParseBlockStatement();

	return WhileExpression::New(token, condition, action);
}

std::shared_ptr<IExpression> Parser::ParseForExpression()
{
	auto token = _currentToken;

	if (!ExpectPeek(TokenType::TOKEN_LPAREN))
	{
		return nullptr;
	}

	NextToken();

	auto init = Parser::ParseStatement();

	if (!CurrentTokenIs(TokenType::TOKEN_SEMICOLON))
	{
		return nullptr;
	}

	NextToken();

	auto condition = ParseExpression(Precedence::LOWEST);

	if (!ExpectPeek(TokenType::TOKEN_SEMICOLON))
	{
		return nullptr;
	}
	
	NextToken();

	auto post = Parser::ParseStatement();

	if (!ExpectPeek(TokenType::TOKEN_RPAREN))
	{
		return nullptr;
	}

	if (!ExpectPeek(TokenType::TOKEN_LBRACE))
	{
		return nullptr;
	}

	//parse the action
	auto action = ParseBlockStatement();

	return ForExpression::New(token, init, condition, post, action);
}

std::shared_ptr<IExpression> Parser::ParseArrayLiteral()
{
	auto token = _currentToken;

	std::vector<std::shared_ptr<IExpression>> arguments;

	while (!PeekTokenIs(TokenType::TOKEN_RBRACKET))
	{
		NextToken();
		if (!CurrentTokenIs(TokenType::TOKEN_COMMA))
		{
			auto arg = ParseExpression(Precedence::LOWEST);
			arguments.emplace_back(arg);
		}
	}

	if (!ExpectPeek(TokenType::TOKEN_RBRACKET))
	{
		return nullptr;
	}

	return ArrayLiteral::New(token, arguments);
}

std::shared_ptr<IExpression> Parser::ParseHashLiteral()
{
	// hashes look like {expression: expression, expression: expression}
	//assume the current is {
	auto token = _currentToken;

	std::map<std::shared_ptr<IExpression>, std::shared_ptr<IExpression>> pairs;

	while (!PeekTokenIs(TokenType::TOKEN_RBRACE))
	{
		NextToken();
		if (!CurrentTokenIs(TokenType::TOKEN_COMMA))
		{
			auto key = ParseExpression(Precedence::LOWEST);

			if (!ExpectPeek(TokenType::TOKEN_COLON))
			{
				return nullptr;
			}

			NextToken();

			auto value = ParseExpression(Precedence::LOWEST);

			pairs[key] = value;
		}
	}
	NextToken();

	return HashLiteral::New(token, pairs);
}

std::shared_ptr<IExpression> Parser::ParseIndexExpression(const std::shared_ptr<IExpression>& left)
{
	NextToken();
	
	auto index = ParseExpression(Precedence::LOWEST);

	if (!ExpectPeek(TokenType::TOKEN_RBRACKET))
	{
		return nullptr;
	}

	return IndexExpression::New(_currentToken, left, index);
}

std::shared_ptr<IExpression> Parser::ParseIncrementExpression(const std::shared_ptr<IExpression>& left)
{
	auto token = _currentToken;
	auto op = _currentToken.Type == TokenType::TOKEN_INCREMENT ? TokenType::TOKEN_PLUS.Name : TokenType::TOKEN_MINUS.Name;

	NextToken();

	return InfixExpression::New(token, left, op, IntegerLiteral::New(token, 1));
}

std::shared_ptr<IStatement> Parser::ParseBlockStatement()
{
	auto token = _currentToken;

	std::vector<std::shared_ptr<IStatement>> statements;

	NextToken();

	while (!CurrentTokenIs(TokenType::TOKEN_RBRACE) && !CurrentTokenIs(TokenType::TOKEN_EOF))
	{
		auto statement = ParseStatement();
		if (statement != nullptr)
		{
			statements.push_back(std::move(statement));
		}
		NextToken();
	}

	return BlockStatement::New(token, statements);
}

std::shared_ptr<IExpression> Parser::ParseFunctionLiteral()
{
	auto token = _currentToken;

	if (!ExpectPeek(TokenType::TOKEN_LPAREN))
	{
		return nullptr;
	}

	std::vector<std::shared_ptr<IExpression>> parameters;

	while (!PeekTokenIs(TokenType::TOKEN_RPAREN))
	{
		NextToken();
		if (CurrentTokenIs(TokenType::TOKEN_IDENT))
		{
			auto param = Identifier::New(_currentToken, _currentToken.Literal);
			parameters.push_back(std::move(param));
		}
	}

	if (!ExpectPeek(TokenType::TOKEN_RPAREN))
	{
		return nullptr;
	}

	if (!ExpectPeek(TokenType::TOKEN_LBRACE))
	{
		return nullptr;
	}

	auto body = ParseBlockStatement();

	return FunctionLiteral::New(token, parameters, body);
}

std::shared_ptr<IExpression> Parser::ParseCallExpression(const std::shared_ptr<IExpression>& function)
{
	auto token = _currentToken;

	std::vector<std::shared_ptr<IExpression>> arguments;

	while (!PeekTokenIs(TokenType::TOKEN_RPAREN))
	{
		NextToken();
		if (!CurrentTokenIs(TokenType::TOKEN_COMMA))
		{
			auto arg = ParseExpression(Precedence::LOWEST);
			arguments.push_back(std::move(arg));
		}
	}

	if (!ExpectPeek(TokenType::TOKEN_RPAREN))
	{
		return nullptr;
	}

	return CallExpression::New(token, function, arguments);
}

std::shared_ptr<IStatement> Parser::ParseLetStatement()
{
	auto token = _currentToken;
	
	NextToken();

	std::shared_ptr<IExpression> holder = nullptr;
	if (_currentToken.Type == TokenType::TOKEN_IDENT)
	{
		holder = Identifier::New(_currentToken, _currentToken.Literal);
	}
	else
	{
		holder = ParseExpression(Precedence::LOWEST);
	}

	if (!ExpectPeek(TokenType::TOKEN_ASSIGN))
	{
		return nullptr;
	}

	NextToken();
	
	auto value = ParseExpression(Precedence::LOWEST);

	if(PeekTokenIs(TokenType::TOKEN_SEMICOLON))
	{
		NextToken();
	}

	return LetStatement::New(token, holder, value);
}

std::shared_ptr<IStatement> Parser::ParseAssignStatement()
{
	auto token = _currentToken;

	std::shared_ptr<IExpression> holder = nullptr;
	if (_currentToken.Type == TokenType::TOKEN_IDENT)
	{
		holder = Identifier::New(_currentToken, _currentToken.Literal);
	}
	else
	{
		holder = ParseExpression(Precedence::LOWEST);
	}

	if (!ExpectPeek(TokenType::TOKEN_ASSIGN))
	{
		return nullptr;
	}

	NextToken();

	auto value = ParseExpression(Precedence::LOWEST);

	if (PeekTokenIs(TokenType::TOKEN_SEMICOLON))
	{
		NextToken();
	}

	return LetStatement::New(Token::New(TokenType::TOKEN_LET, "let"), holder, value);
}

std::shared_ptr<IStatement> Parser::ParseReturnStatement()
{
	auto token = _currentToken;

	NextToken();

	auto returnValue = ParseExpression(Precedence::LOWEST);

	while (_currentToken.Type != TokenType::TOKEN_SEMICOLON)
	{
		NextToken();
	}

	return ReturnStatement::New(token, returnValue);
}

std::shared_ptr<IStatement> Parser::ParseBreakStatement()
{
	auto token = _currentToken;

	while (_currentToken.Type != TokenType::TOKEN_SEMICOLON)
	{
		NextToken();
	}

	return BreakStatement::New(token);
}

std::shared_ptr<IStatement> Parser::ParseContinueStatement()
{
	auto token = _currentToken;

	while (_currentToken.Type != TokenType::TOKEN_SEMICOLON)
	{
		NextToken();
	}

	return ContinueStatement::New(token);
}

std::shared_ptr<IStatement> Parser::ParseExpressionStatement()
{
	auto token = _currentToken;

	//parse the expression
	auto expression = ParseExpression(Precedence::LOWEST);

	//if the next token is a semicolon, consume it
	if (PeekTokenIs(TokenType::TOKEN_SEMICOLON))
	{
		NextToken();
	}

	return ExpressionStatement::New(token, expression);
}


bool Parser::ExpectPeek(const TokenType& expectedType)
{
	if (_nextToken.Type == expectedType)
	{
		NextToken();
		return true;
	}
	
	PeekError(expectedType);
	return false;
}

bool Parser::PeekTokenIs(const TokenType& expectedType) const
{
	return _nextToken.Type == expectedType;
}

bool Parser::CurrentTokenIs(const TokenType& expectedType) const
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

void Parser::PeekError(const TokenType& expectedType)
{
	std::string error = "expected next token to be " + expectedType.Name + ", got " + _nextToken.Type.Name + " instead";
	AddError(error);
}

void Parser::RegisterInfix(const TokenType& type, const InfixParseFn& fn)
{
	_infixDispatch[type] = fn;
}

void Parser::RegisterPrefix(const TokenType& type, const PrefixParseFn& fn)
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