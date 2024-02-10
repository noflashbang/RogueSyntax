#include "Lexer.h"

Lexer::Lexer(const std::string& input) : _input(input), _position(0), _readPosition(0), _line(1), _character(0), _column(0)
{
	ReadChar();
}

Token Lexer::NextToken()
{
	Token token = Token::New();
	SkipWhitespace();

	TokenLocation location = { _line, _character, _column, "" };
	switch (_currentChar)
	{
		case '=':
		{
			token = Token::New(TokenType::TOKEN_ASSIGN, _currentChar);
			break;
		}
		case ';':
		{
			token = Token::New(TokenType::TOKEN_SEMICOLON, _currentChar);
			break;
		}
		case '(':
		{
			token = Token::New(TokenType::TOKEN_LPAREN, _currentChar);
			break;
		}
		case ')':
		{
			token = Token::New(TokenType::TOKEN_RPAREN, _currentChar);
			break;
		}
		case ',':
		{
			token = Token::New(TokenType::TOKEN_COMMA, _currentChar);
			break;
		}
		case '+':
		{
			token = Token::New(TokenType::TOKEN_PLUS, _currentChar);
			break;
		}
		case '{':
		{
			token = Token::New(TokenType::TOKEN_LBRACE, _currentChar);
			break;
		}
		case '}':
		{
			token = Token::New(TokenType::TOKEN_RBRACE, _currentChar);
			break;
		}
		case '\0':
		{
			token = Token::New(TokenType::TOKEN_EOF, "");
			break;
		}
		default:
		{
			if(IsLetter(_currentChar))
			{
				auto literal = ReadIdentifier();
				auto tokenType = TokenType::LookupIdent(literal);
				token = Token::New(tokenType, literal);
				token.Location = location;
				return token;
			}
			else if (IsDigit(_currentChar))
			{
				auto literal = ReadNumber();
				token = Token::New(TokenType::TOKEN_INT, literal);
				token.Location = location;
				return token;
			}
			else
			{
				token = Token::New(TokenType::TOKEN_ILLEGAL, _currentChar);
			}

			break;
		}
	}

	ReadChar();

	token.Location = location;
	return token;
}

void Lexer::ReadChar()
{
	if (_position >= std::size(_input))
	{
		_currentChar = '\0';
	}
	else
	{
		_currentChar = _input[_readPosition];
	}
	_position = _readPosition;
	_readPosition++;

	if ('\n' == _currentChar)
	{
		_line++;
		_column = 0;
		_character = 0;
	}
	else
	{
		if ('\t' == _currentChar)
		{
			_column += 4;
		}
		else
		{
			_column++;
		}

		_character++;
	}
}

std::string Lexer::ReadIdentifier()
{
	auto position = _position;
	auto cnt = 0;
	while (IsLetter(_currentChar))
	{
		cnt++;
		ReadChar();
	}
	return _input.substr(position, cnt);
}

std::string Lexer::ReadNumber()
{
	auto position = _position;
	auto cnt = 0;
	while (IsDigit(_currentChar))
	{
		cnt++;
		ReadChar();
	}
	return _input.substr(position, cnt);
}

void Lexer::SkipWhitespace()
{
	while (IsWhitespace(_currentChar))
	{
		ReadChar();
	}
}

bool Lexer::IsLetter(char c)
{
	return std::isalpha(c) || c == '_';
}

bool Lexer::IsDigit(char c)
{
	return std::isdigit(c);
}

bool Lexer::IsWhitespace(char c)
{
	return std::isspace(c);
}