#include "Lexer.h"
#include "pch.h"

Lexer::Lexer(const std::string& input) : _input(input), _position(0), _readPosition(0), _currentChar('\0'), _peekChar('\0')
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
	Token result = Token::New();
	switch (_currentChar)
	{
		case '=':
		{
			if (_peekChar == '=')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_EQ, std::string(1, ch) + _currentChar);
			}
			else
			{
				result = Token::New(TokenType::TOKEN_ASSIGN, _currentChar);
			}
			break;
		}
		case ';':
		{
			result = Token::New(TokenType::TOKEN_SEMICOLON, _currentChar);
			break;
		}
		case ':':
		{
			result = Token::New(TokenType::TOKEN_COLON, _currentChar);
			break;
		}
		case '(':
		{
			result = Token::New(TokenType::TOKEN_LPAREN, _currentChar);
			break;
		}
		case ')':
		{
			result = Token::New(TokenType::TOKEN_RPAREN, _currentChar);
			break;
		}
		case ',':
		{
			result = Token::New(TokenType::TOKEN_COMMA, _currentChar);
			break;
		}
		case '+':
		{
			if (_peekChar == '+')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_INCREMENT, std::string(1, ch) + _currentChar);
			}
			else if (_peekChar == '=')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_PLUS_ASSIGN, std::string(1, ch) + _currentChar);	
			}
			else
			{
				result = Token::New(TokenType::TOKEN_PLUS, _currentChar);
			}
			break;
		}
		case '-':
		{
			if (_peekChar == '-')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_DECREMENT, std::string(1, ch) + _currentChar);
			}
			else if (_peekChar == '=')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_MINUS_ASSIGN, std::string(1, ch) + _currentChar);
			}
			else
			{
				result = Token::New(TokenType::TOKEN_MINUS, _currentChar);
			}
			break;
		}
		case '!':
		{
			if (_peekChar == '=')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_NOT_EQ, std::string(1, ch) + _currentChar);
			}
			else
			{
				result = Token::New(TokenType::TOKEN_BANG, _currentChar);
			}
			break;
		}
		case '/':
		{
			if (_peekChar == '=')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_SLASH_ASSIGN, std::string(1, ch) + _currentChar);
			}
			else
			{
				result = Token::New(TokenType::TOKEN_SLASH, _currentChar);
			}
			break;
		}
		case '*':
		{
			if (_peekChar == '=')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_ASTERISK_ASSIGN, std::string(1, ch) + _currentChar);
			}
			else
			{
				result = Token::New(TokenType::TOKEN_ASTERISK, _currentChar);
			}
			break;
		}
		case '<':
		{
			if (_peekChar == '=')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_LT_EQ, std::string(1, ch) + _currentChar);
			}
			else if (_peekChar == '<')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_SHIFT_LEFT, std::string(1, ch) + _currentChar);
			}
			else
			{
				result = Token::New(TokenType::TOKEN_LT, _currentChar);
			}
			break;
		}
		case '>':
		{
			if (_peekChar == '=')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_GT_EQ, std::string(1, ch) + _currentChar);
			}
			else if (_peekChar == '>')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_SHIFT_RIGHT, std::string(1, ch) + _currentChar);
			}
			else
			{
				result = Token::New(TokenType::TOKEN_GT, _currentChar);
			}
			break;
		}
		case '&':
		{
			if (_peekChar == '&')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_AND, std::string(1, ch) + _currentChar);
			}
			else
			{
				result = Token::New(TokenType::TOKEN_BITWISE_AND, _currentChar);
			}
			break;
		}
		case '|':
		{
			if (_peekChar == '|')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_OR, std::string(1, ch) + _currentChar);
			}
			else
			{
				result = Token::New(TokenType::TOKEN_BITWISE_OR, _currentChar);
			}
			break;
		}
		case '^':
		{
			result = Token::New(TokenType::TOKEN_BITWISE_XOR, _currentChar);
			break;
		}
		case '~':
		{
			result = Token::New(TokenType::TOKEN_BITWISE_NOT, _currentChar);
			break;
		}
		case '%':
		{
			if (_peekChar == '=')
			{
				auto ch = _currentChar;
				ReadChar();
				result = Token::New(TokenType::TOKEN_MODULO_ASSIGN, std::string(1, ch) + _currentChar);
			}
			else
			{
				result = Token::New(TokenType::TOKEN_MODULO, _currentChar);
			}
			break;
		}
		case '{':
		{
			result = Token::New(TokenType::TOKEN_LBRACE, _currentChar);
			break;
		}
		case '}':
		{
			result = Token::New(TokenType::TOKEN_RBRACE, _currentChar);
			break;
		}
		case '[':
		{
			result = Token::New(TokenType::TOKEN_LBRACKET, _currentChar);
			break;
		}
		case ']':
		{
			result = Token::New(TokenType::TOKEN_RBRACKET, _currentChar);
			break;
		}
		case '\0':
		{
			result = Token::New(TokenType::TOKEN_EOF, "");
			break;
		}
		case '"':
		{
			result = Token::New(TokenType::TOKEN_STRING, ReadString());
			break;
		}
		default:
		{
			if (IsLetter(_currentChar))
			{
				auto literal = ReadIdentifier();
				auto tokenType = TokenType::LookupIdent(literal);
				result = Token::New(tokenType, literal);
			}
			else if (IsDigit(_currentChar))
			{
				auto literal = ReadNumber();
				if (literal.ends_with('d') || std::any_of(literal.begin(), literal.end(), [](char c) { return c == '.'; }))
				{
					result = Token::New(TokenType::TOKEN_DECIMAL, literal);
				}
				else
				{
					result = Token::New(TokenType::TOKEN_INT, literal);
				}
			}
			else
			{
				result = Token::New(TokenType::TOKEN_ILLEGAL, _currentChar);
			}
		}
	}
	return result;
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
	while (IsDigit(_peekChar) || _peekChar == '.' || _peekChar == 'd')
	{
		cnt++;
		ReadChar();
	}
	return _input.substr(position, cnt);
}

std::string Lexer::ReadString()
{
	//TODO: handle escape characters

	auto position = _position;
	auto cnt = 1;

	while (_peekChar != '"')
	{
		cnt++;
		ReadChar();
	}
	cnt++;
	ReadChar();

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
	return (std::isalpha(c) != 0) || (c == '_');
}

bool Lexer::IsDigit(const char c) const
{
	return (std::isdigit(c) != 0);
}

bool Lexer::IsWhitespace(const char c) const
{
	return (std::isspace(c) != 0);
}