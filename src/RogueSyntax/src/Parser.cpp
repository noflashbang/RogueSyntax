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
	_prefixDispatch[TokenType::TOKEN_BITWISE_NOT] = std::bind(&Parser::ParsePrefixExpression, this);
	_prefixDispatch[TokenType::TOKEN_FALSE] = std::bind(&Parser::ParseBoolean, this);
	_prefixDispatch[TokenType::TOKEN_TRUE] = std::bind(&Parser::ParseBoolean, this);
	_prefixDispatch[TokenType::TOKEN_LPAREN] = std::bind(&Parser::ParseGroupedExpression, this);
	_prefixDispatch[TokenType::TOKEN_FUNCTION] = std::bind(&Parser::ParseFunctionLiteral, this);
	_prefixDispatch[TokenType::TOKEN_LBRACKET] = std::bind(&Parser::ParseArrayLiteral, this);
	_prefixDispatch[TokenType::TOKEN_LBRACE] = std::bind(&Parser::ParseHashLiteral, this);

	//register infix operators
	_infixDispatch[TokenType::TOKEN_PLUS] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_MINUS] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_SLASH] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_MODULO] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_ASTERISK] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_EQ] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_NOT_EQ] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_LT] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_GT] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_AND] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_OR] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_BITWISE_AND] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_BITWISE_OR] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_BITWISE_XOR] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_SHIFT_LEFT] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_SHIFT_RIGHT] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_LT_EQ] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_GT_EQ] = std::bind(&Parser::ParseInfixExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_LPAREN] = std::bind(&Parser::ParseCallExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_LBRACKET] = std::bind(&Parser::ParseIndexExpression, this, std::placeholders::_1);

	_infixDispatch[TokenType::TOKEN_ASSIGN] = std::bind(&Parser::ParseAssignExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_INCREMENT] = std::bind(&Parser::ParseOpAssignExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_DECREMENT] = std::bind(&Parser::ParseOpAssignExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_PLUS_ASSIGN] = std::bind(&Parser::ParseOpAssignExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_MINUS_ASSIGN] = std::bind(&Parser::ParseOpAssignExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_ASTERISK_ASSIGN] = std::bind(&Parser::ParseOpAssignExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_SLASH_ASSIGN] = std::bind(&Parser::ParseOpAssignExpression, this, std::placeholders::_1);
	_infixDispatch[TokenType::TOKEN_MODULO_ASSIGN] = std::bind(&Parser::ParseOpAssignExpression, this, std::placeholders::_1);

	//load the first two tokens
	NextToken();
	NextToken();
}

void Parser::NextToken()
{
	_currentToken = _nextToken;
	_nextToken = _lexer.NextToken();
}

std::shared_ptr<Program> Parser::ParseProgram(const std::string& sourceName)
{
	_currentStore = std::make_shared<AstNodeStore>();
	auto program = Program::New(_currentStore, sourceName);

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

IStatement* Parser::ParseStatement()
{
	IStatement* statement = nullptr;
	if (_currentToken.Type == TokenType::TOKEN_LET)
	{
		statement = ParseLetStatement();
	}
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
	else if (_currentToken.Type == TokenType::TOKEN_IF)
	{
		statement = ParseIfStatement();
	}
	else if (_currentToken.Type == TokenType::TOKEN_FOR)
	{
		statement = ParseForStatement();
	}
	else if (_currentToken.Type == TokenType::TOKEN_WHILE)
	{
		statement = ParseWhileStatement();
	}
	else
	{
		statement = ParseExpressionStatement();
	}

	return statement;
}

IExpression* Parser::ParseExpression(const Precedence precedence)
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

IExpression* Parser::ParseIdentifier()
{
	return _currentStore->New_Identifier(_currentToken, _currentToken.Literal);
}

IExpression* Parser::ParseNullLiteral()
{
	return _currentStore->New_NullLiteral(_currentToken);
}

IExpression* Parser::ParseIntegerLiteral()
{
	try
	{
		return _currentStore->New_IntegerLiteral(_currentToken, std::stoi(_currentToken.Literal));
	}
	catch (const std::invalid_argument& e)
	{
		std::string error = "could not parse " + _currentToken.Literal + " as integer";
		AddError(error);
		AddError(e.what());
		return nullptr;
	}
}

IExpression* Parser::ParseDecimalLiteral()
{
	try
	{
		//remove the trailing d
		if (_currentToken.Literal.ends_with('d'))
		{
			_currentToken.Literal.pop_back();
		}
		return _currentStore->New_DecimalLiteral(_currentToken, std::stof(_currentToken.Literal));
	}
	catch (const std::invalid_argument& e)
	{
		std::string error = "could not parse " + _currentToken.Literal + " as decimal";
		AddError(error);
		AddError(e.what());
		return nullptr;
	}
}

IExpression* Parser::ParseStringLiteral()
{
	//remove the quotes
	std::string result(_currentToken.Literal.begin() + 1, _currentToken.Literal.end() - 1);

	return _currentStore->New_StringLiteral(_currentToken, result);
}

IExpression* Parser::ParsePrefixExpression()
{
	auto token = _currentToken;
	auto op = _currentToken.Literal;

	NextToken();

	auto right = ParseExpression(Precedence::PREFIX);

	return _currentStore->New_PrefixExpression(token, op, right);
}

IExpression* Parser::ParseInfixExpression(const IExpression* left)
{	
	auto token = _currentToken;
	auto op = _currentToken.Literal;

	Precedence precedence = CurrentPrecedence();
	NextToken();
	auto right = ParseExpression(precedence);

	return _currentStore->New_InfixExpression(token, left, op, right);
}

IExpression* Parser::ParseBoolean()
{
	return _currentStore->New_BooleanLiteral(_currentToken, _currentToken.Type == TokenType::TOKEN_TRUE);
}

IExpression* Parser::ParseGroupedExpression()
{
	NextToken();

	auto expression = ParseExpression(Precedence::LOWEST);

	if (!ExpectPeek(TokenType::TOKEN_RPAREN))
	{
		return nullptr;
	}

	return expression;
}

IStatement* Parser::ParseIfStatement()
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

	IStatement* alternative = nullptr;

	if (PeekTokenIs(TokenType::TOKEN_ELSE))
	{
		NextToken();

		if (!ExpectPeek(TokenType::TOKEN_LBRACE))
		{
			return nullptr;
		}
		auto alt = ParseBlockStatement();
		alternative = alt;
	}

	return _currentStore->New_IfStatement(token, condition, consequence, alternative);
}

IStatement* Parser::ParseWhileStatement()
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

	return _currentStore->New_WhileStatement(token, condition, action);
}

IStatement* Parser::ParseForStatement()
{
	auto token = _currentToken;

	if (!ExpectPeek(TokenType::TOKEN_LPAREN))
	{
		return nullptr;
	}

	NextToken();

	auto init = Parser::ParseStatement();

	NextToken();

	auto condition = ParseExpression(Precedence::LOWEST);
	
	NextToken();
	NextToken();

	auto post = Parser::ParseStatement();

	//if (!ExpectPeek(TokenType::TOKEN_RPAREN))
	//{
	//	return nullptr;
	//}

	if (!ExpectPeek(TokenType::TOKEN_LBRACE))
	{
		return nullptr;
	}

	//parse the action
	auto action = ParseBlockStatement();

	return _currentStore->New_ForStatement(token, init, condition, post, action);
}

IExpression* Parser::ParseArrayLiteral()
{
	auto token = _currentToken;

	std::vector<IExpression*> arguments;

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

	return _currentStore->New_ArrayLiteral(token, arguments);
}

IExpression* Parser::ParseHashLiteral()
{
	// hashes look like {expression: expression, expression: expression}
	//assume the current is {
	auto token = _currentToken;

	std::map<IExpression*, IExpression*> pairs;

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

	return _currentStore->New_HashLiteral(token, pairs);
}

IExpression* Parser::ParseIndexExpression(const IExpression* left)
{
	NextToken();
	
	auto index = ParseExpression(Precedence::LOWEST);

	if (!ExpectPeek(TokenType::TOKEN_RBRACKET))
	{
		return nullptr;
	}

	return _currentStore->New_IndexExpression(_currentToken, left, index);
}

IExpression* Parser::ParseAssignExpression(const IExpression* left)
{
	auto token = left->BaseToken;
	token.Literal = "let";
	NextToken();

	auto right = ParseExpression(Precedence::LOWEST);

	if (PeekTokenIs(TokenType::TOKEN_SEMICOLON))
	{
		NextToken();
	}
	else
	{
		if (PeekTokenIs(TokenType::TOKEN_RPAREN))
		{
			NextToken();
		}
		else
		{
			return nullptr;
		}
	}
	
	return _currentStore->New_LetStatement(token, left, right);
}

IExpression* Parser::ParseOpAssignExpression(const IExpression* left)
{
	auto token = _currentToken;
	NextToken();
	if (token.Type == TokenType::TOKEN_INCREMENT)
	{
		Token opToken = Token(TokenType::TOKEN_PLUS, "+");
		opToken.Location = token.Location;

		auto right = _currentStore->New_InfixExpression(opToken, left, opToken.Literal, _currentStore->New_IntegerLiteral(opToken, 1));

		token.Literal = "let";
		return _currentStore->New_LetStatement(token, left, right);
	}
	else if (token.Type == TokenType::TOKEN_DECREMENT)
	{
		Token opToken = Token(TokenType::TOKEN_MINUS, "-");
		opToken.Location = token.Location;
		auto right = _currentStore->New_InfixExpression(opToken, left, opToken.Literal, _currentStore->New_IntegerLiteral(opToken, 1));

		token.Literal = "let";
		return _currentStore->New_LetStatement(token, left, right);
	}
	else if (token.Type == TokenType::TOKEN_MINUS_ASSIGN)
	{
		Token opToken = Token(TokenType::TOKEN_MINUS, "-");
		opToken.Location = token.Location;
		auto right = _currentStore->New_InfixExpression(opToken, left, opToken.Literal, ParseExpression(Precedence::LOWEST));

		token.Literal = "let";
		return _currentStore->New_LetStatement(token, left, right);
	}
	else if (token.Type == TokenType::TOKEN_PLUS_ASSIGN)
	{
		Token opToken = Token(TokenType::TOKEN_PLUS, "+");
		opToken.Location = token.Location;
		auto right = _currentStore->New_InfixExpression(opToken, left, opToken.Literal, ParseExpression(Precedence::LOWEST));

		token.Literal = "let";
		return _currentStore->New_LetStatement(token, left, right);
	}
	else if (token.Type == TokenType::TOKEN_ASTERISK_ASSIGN)
	{
		Token opToken = Token(TokenType::TOKEN_ASTERISK, "*");
		opToken.Location = token.Location;
		auto right = _currentStore->New_InfixExpression(opToken, left, opToken.Literal, ParseExpression(Precedence::LOWEST));

		token.Literal = "let";
		return _currentStore->New_LetStatement(token, left, right);
	}
	else if (token.Type == TokenType::TOKEN_SLASH_ASSIGN)
	{
		Token opToken = Token(TokenType::TOKEN_SLASH, "/");
		opToken.Location = token.Location;
		auto right = _currentStore->New_InfixExpression(opToken, left, opToken.Literal, ParseExpression(Precedence::LOWEST));

		token.Literal = "let";
		return _currentStore->New_LetStatement(token, left, right);
	}
	else if (token.Type == TokenType::TOKEN_MODULO_ASSIGN)
	{
		Token opToken = Token(TokenType::TOKEN_MODULO, "%");
		opToken.Location = token.Location;
		auto right = _currentStore->New_InfixExpression(opToken, left, opToken.Literal, ParseExpression(Precedence::LOWEST));

		token.Literal = "let";
		return _currentStore->New_LetStatement(token, left, right);
	}
	else
	{
		std::string error = "unknown operator " + token.Literal;
		AddError(error);
		return nullptr;
	}
}

IStatement* Parser::ParseBlockStatement()
{
	auto token = _currentToken;

	std::vector<IStatement*> statements;

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

	return _currentStore->New_BlockStatement(token, statements);
}

IExpression* Parser::ParseFunctionLiteral()
{
	auto token = _currentToken;

	if (!ExpectPeek(TokenType::TOKEN_LPAREN))
	{
		return nullptr;
	}

	std::vector<IExpression*> parameters;

	while (!PeekTokenIs(TokenType::TOKEN_RPAREN))
	{
		NextToken();
		if (CurrentTokenIs(TokenType::TOKEN_IDENT))
		{
			auto param = _currentStore->New_Identifier(_currentToken, _currentToken.Literal);
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

	return _currentStore->New_FunctionLiteral(token, parameters, body);
}

IExpression* Parser::ParseCallExpression(const IExpression* function)
{
	auto token = _currentToken;

	std::vector<IExpression*> arguments;

	if (function == nullptr)
	{
		std::string error = "CallExpression can not have NULL function; Token :" + token.Literal + "";
		AddError(error);
		return nullptr;
	}

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

	return _currentStore->New_CallExpression(token, function, arguments);
}

IStatement* Parser::ParseLetStatement()
{
	auto token = _currentToken;
	
	NextToken();

	IExpression* holder = nullptr;
	if (_currentToken.Type == TokenType::TOKEN_IDENT && _nextToken.Type != TokenType::TOKEN_LBRACKET)
	{
		holder = _currentStore->New_Identifier(_currentToken, _currentToken.Literal);
	}
	else
	{
		holder = ParseExpression(Precedence::LOWEST);
		if (holder != NULL && holder->IsThisA<LetStatement>())
		{
			return dynamic_cast<LetStatement*>(holder);
		}
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
	else
	{
		if (PeekTokenIs(TokenType::TOKEN_RPAREN))
		{
			NextToken();
		}
		else
		{
			return nullptr;
		}
	}

	if (value->Type() == TokenType::TOKEN_FUNCTION)
	{
		auto function = dynamic_cast<FunctionLiteral*>(value);
		function->Name = dynamic_cast<Identifier*>(holder)->Value;
	}

	return _currentStore->New_LetStatement(token, holder, value);
}

IStatement* Parser::ParseReturnStatement()
{
	auto token = _currentToken;

	NextToken();

	auto returnValue = ParseExpression(Precedence::LOWEST);

	while (_currentToken.Type != TokenType::TOKEN_SEMICOLON)
	{
		NextToken();
	}

	return _currentStore->New_ReturnStatement(token, returnValue);
}

IStatement* Parser::ParseBreakStatement()
{
	auto token = _currentToken;

	while (_currentToken.Type != TokenType::TOKEN_SEMICOLON)
	{
		NextToken();
	}

	return _currentStore->New_BreakStatement(token);
}

IStatement* Parser::ParseContinueStatement()
{
	auto token = _currentToken;

	while (_currentToken.Type != TokenType::TOKEN_SEMICOLON)
	{
		NextToken();
	}

	return _currentStore->New_ContinueStatement(token);
}

IStatement* Parser::ParseExpressionStatement()
{
	auto token = _currentToken;

	//if the current token is a semicolon, consume it
	while (_currentToken.Type == TokenType::TOKEN_SEMICOLON)
	{
		NextToken();
		token = _currentToken;
	}

	//parse the expression
	auto expression = ParseExpression(Precedence::LOWEST);
	if (expression == nullptr)
	{
		return nullptr;
	}

	//if the next token is a semicolon, consume it
	while (PeekTokenIs(TokenType::TOKEN_SEMICOLON))
	{
		NextToken();
	}

	if (expression->IsThisA<LetStatement>())
	{
		return dynamic_cast<LetStatement*>(expression);
	}

	return _currentStore->New_ExpressionStatement(token, expression);
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