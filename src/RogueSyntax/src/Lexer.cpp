#include "Lexer.h"

Lexer::Lexer(const std::string& input) : _input(input), _position(0), _readPosition(0)
{
	_currentLocation.Reset();
	ReadChar();
}

Token Lexer::NextToken()
{
	SkipWhitespace();

	//save the start position of the token
	TokenLocation location = _currentLocation;

	auto token = GetCurrentToken();
	token.Location = location;

	ReadChar();

	return token;
}

Token Lexer::GetCurrentToken()
{
	switch (_currentChar)
	{
		case '=':
		{
			if (_peekChar == '=')
			{
				auto ch = _currentChar;
				ReadChar();
				return Token::New(TokenType::TOKEN_EQ, std::string(1, ch) + _currentChar);
			}
			else
			{
				return Token::New(TokenType::TOKEN_ASSIGN, _currentChar);
			}
		}
		case ';':
		{
			return Token::New(TokenType::TOKEN_SEMICOLON, _currentChar);
		}
		case '(':
		{
			return Token::New(TokenType::TOKEN_LPAREN, _currentChar);
		}
		case ')':
		{
			return Token::New(TokenType::TOKEN_RPAREN, _currentChar);
		}
		case ',':
		{
			return Token::New(TokenType::TOKEN_COMMA, _currentChar);
		}
		case '+':
		{
			if (_peekChar == '+')
			{
				auto ch = _currentChar;
				ReadChar();
				return Token::New(TokenType::TOKEN_INCREMENT, std::string(1, ch) + _currentChar);
			}
			else
			{
				return Token::New(TokenType::TOKEN_PLUS, _currentChar);
			}
		}
		case '-':
		{
			if (_peekChar == '-')
			{
				auto ch = _currentChar;
				ReadChar();
				return Token::New(TokenType::TOKEN_DECREMENT, std::string(1, ch) + _currentChar);
			}
			else
			{
				return Token::New(TokenType::TOKEN_MINUS, _currentChar);
			}
		}
		case '!':
		{
			if (_peekChar == '=')
			{
				auto ch = _currentChar;
				ReadChar();
				return Token::New(TokenType::TOKEN_NOT_EQ, std::string(1, ch) + _currentChar);
			}
			else
			{
				return Token::New(TokenType::TOKEN_BANG, _currentChar);
			}
		}
		case '/':
		{
			return Token::New(TokenType::TOKEN_SLASH, _currentChar);
		}
		case '*':
		{
			return Token::New(TokenType::TOKEN_ASTERISK, _currentChar);
		}
		case '<':
		{
			return Token::New(TokenType::TOKEN_LT, _currentChar);
		}
		case '>':
		{
			return Token::New(TokenType::TOKEN_GT, _currentChar);
		}
		case '{':
		{
			return Token::New(TokenType::TOKEN_LBRACE, _currentChar);
		}
		case '}':
		{
			return Token::New(TokenType::TOKEN_RBRACE, _currentChar);
		}
		case '\0':
		{
			return Token::New(TokenType::TOKEN_EOF, "");
		}
		default:
		{
			if (IsLetter(_currentChar))
			{
				auto literal = ReadIdentifier();
				auto tokenType = TokenType::LookupIdent(literal);
				return Token::New(tokenType, literal);
			}
			else if (IsDigit(_currentChar))
			{
				auto literal = ReadNumber();
				return Token::New(TokenType::TOKEN_INT, literal);
			}
			else
			{
				return Token::New(TokenType::TOKEN_ILLEGAL, _currentChar);
			}
		}
	}
	return Token::New();
}

void Lexer::ReadChar()
{
	_currentChar = _readPosition >= std::size(_input) ? '\0' : _input[_readPosition];
	_peekChar = (_readPosition + 1) >= std::size(_input) ? '\0' : _input[(_readPosition + 1)];
	_position = _readPosition;
	_readPosition++;
	_currentLocation.Advance(_currentChar);
}

std::string Lexer::ReadIdentifier()
{
	auto position = _position;
	auto cnt = 1;

	while (IsLetter(_peekChar))
	{
		cnt++;
		ReadChar();
	}
	return _input.substr(position, cnt);
}

std::string Lexer::ReadNumber()
{
	auto position = _position;
	auto cnt = 1;
	while (IsDigit(_peekChar))
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

bool Lexer::IsLetter(const char c) const
{
	return std::isalpha(c) || c == '_';
}

bool Lexer::IsDigit(const char c) const
{
	return std::isdigit(c);
}

bool Lexer::IsWhitespace(const char c) const
{
	return std::isspace(c);
}