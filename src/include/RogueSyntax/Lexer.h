#pragma once

#include "StandardLib.h"
#include "Token.h"

class Lexer
{
public:
	Lexer(const std::string& input);
	Token NextToken(bool skipComments);

private:

	Token GetCurrentToken();

	void ReadChar();
	std::string ReadIdentifier();
	std::string ReadNumber();
	std::string ReadString();
	std::string ReadComment();

	void SkipWhitespace();

	bool IsLetter(const char c) const;
	bool IsDigit(const char c) const;
	bool IsWhitespace(const char c) const;

	std::string _input;
	unsigned int _position; //current position in input (points to current char)
	unsigned int _readPosition; //current reading position in input (after current char)
	char _currentChar;
	char _peekChar;

	//current line and column
	TokenLocation _currentLocation;
};
