#pragma once

#include "StandardLib.h"
#include "Token.h"

class Lexer
{
public:
	Lexer(const std::string& input);
	Token NextToken();

private:

	void ReadChar();
	std::string ReadIdentifier();
	std::string ReadNumber();
	void SkipWhitespace();

	bool IsLetter(char c);
	bool IsDigit(char c);
	bool IsWhitespace(char c);

	std::string _input;
	unsigned int _position; //current position in input (points to current char)
	unsigned int _readPosition; //current reading position in input (after current char)
	char _currentChar;

	//current line and column
	unsigned int _line;
	unsigned int _character;
	unsigned int _column;
};
