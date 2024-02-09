#include "Lexer.h"

Lexer::Lexer(const std::string& input) : _input(input), _position(0), _readPosition(0)
{
	ReadChar();
}

Token::Token Lexer::NextToken()
{
	Token::Token token;

	SkipWhitespace();

	switch (_currentChar)
	{
		case '=':
		{
			token = Token::Token(Token::TokenType::RS_ASSIGN, _currentChar);
			break;
		}
		case ';':
		{
			token = Token::Token(Token::TokenType::RS_SEMICOLON, _currentChar);
			break;
		}
		case '(':
		{
			token = Token::Token(Token::TokenType::RS_LPAREN, _currentChar);
			break;
		}
		case ')':
		{
			token = Token::Token(Token::TokenType::RS_RPAREN, _currentChar);
			break;
		}
		case ',':
		{
			token = Token::Token(Token::TokenType::RS_COMMA, _currentChar);
			break;
		}
		case '+':
		{
			token = Token::Token(Token::TokenType::RS_PLUS, _currentChar);
			break;
		}
		case '{':
		{
			token = Token::Token(Token::TokenType::RS_LBRACE, _currentChar); 
			break;
		}
		case '}':
		{
			token = Token::Token(Token::TokenType::RS_RBRACE, _currentChar);
			break;
		}
		case '\0':
		{
			token = Token::Token(Token::TokenType::RS_EOF, "");
			break;
		}
		default:
		{
			if(IsLetter(_currentChar))
			{
				auto literal = ReadIdentifier();
				auto tokenType = Token::Token::LookupIdent(literal);
				token = Token::Token(tokenType, literal);
				return token;
			}
			else if (IsDigit(_currentChar))
			{
				auto literal = ReadNumber();
				token = Token::Token(Token::TokenType::RS_INT, literal);
				return token;
			}
			else
			{
				token = Token::Token(Token::TokenType::RS_ILLEGAL, _currentChar);
			}

			break;
		}
	}

	ReadChar();
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