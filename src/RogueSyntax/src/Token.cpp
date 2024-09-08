#include "Token.h"

// Special tokens
const TokenType TokenType::TOKEN_ILLEGAL   = { 0,  "ILLEGAL" };
const TokenType TokenType::TOKEN_EOF       = { 1,  "EOF" };
										   		   
// Identifiers + literals				   		   
const TokenType TokenType::TOKEN_IDENT     = { 2,  "IDENT" };
const TokenType TokenType::TOKEN_INT       = { 3,  "INT" };
										   		   
// Operators							   		   
const TokenType TokenType::TOKEN_ASSIGN    = { 4,  "=" };
const TokenType TokenType::TOKEN_PLUS      = { 5,  "+" };
const TokenType TokenType::TOKEN_INCREMENT = { 6,  "++" };
const TokenType TokenType::TOKEN_MINUS     = { 7,  "-" };
const TokenType TokenType::TOKEN_DECREMENT = { 8,  "--" };
const TokenType TokenType::TOKEN_BANG      = { 9,  "!" };
const TokenType TokenType::TOKEN_ASTERISK  = { 10,  "*" };
const TokenType TokenType::TOKEN_SLASH     = { 11,  "/" };
const TokenType TokenType::TOKEN_LT        = { 12, "<" };
const TokenType TokenType::TOKEN_GT        = { 13, ">" };
const TokenType TokenType::TOKEN_EQ        = { 14, "==" };
const TokenType TokenType::TOKEN_NOT_EQ    = { 15, "!=" };

// Delimiters							   		   
const TokenType TokenType::TOKEN_COMMA     = { 16,  "," };
const TokenType TokenType::TOKEN_SEMICOLON = { 17,  ";" };
												   
const TokenType TokenType::TOKEN_LPAREN    = { 18,  "(" };
const TokenType TokenType::TOKEN_RPAREN    = { 19,  ")" };
const TokenType TokenType::TOKEN_LBRACE    = { 20, "{" };
const TokenType TokenType::TOKEN_RBRACE    = { 21, "}" };

// Keywords
const TokenType TokenType::TOKEN_FUNCTION  = { 22, "FUNCTION" };
const TokenType TokenType::TOKEN_LET       = { 23, "LET" };
const TokenType TokenType::TOKEN_TRUE      = { 24, "TRUE" };
const TokenType TokenType::TOKEN_FALSE     = { 25, "FALSE" };
const TokenType TokenType::TOKEN_IF        = { 26, "IF" };
const TokenType TokenType::TOKEN_ELSE      = { 27, "ELSE" };
const TokenType TokenType::TOKEN_RETURN    = { 28, "RETURN" };
const TokenType TokenType::TOKEN_WHILE     = { 29, "WHILE" };
const TokenType TokenType::TOKEN_BREAK     = { 30, "BREAK" };
const TokenType TokenType::TOKEN_CONTINUE  = { 31, "CONTINUE" };
const TokenType TokenType::TOKEN_FOR       = { 32, "FOR" };



const std::map<std::string, TokenType> TokenType::KEYWORD_TOKEN_MAP = {
	{"fn",  TokenType::TOKEN_FUNCTION},
	{"let", TokenType::TOKEN_LET},
	{"true", TokenType::TOKEN_TRUE},
	{"false", TokenType::TOKEN_FALSE},
	{"if", TokenType::TOKEN_IF},
	{"else", TokenType::TOKEN_ELSE},
	{"return", TokenType::TOKEN_RETURN},
	{"while", TokenType::TOKEN_WHILE},
	{"break", TokenType::TOKEN_BREAK},
	{"continue", TokenType::TOKEN_CONTINUE},
	{"for", TokenType::TOKEN_FOR},
};

void TokenLocation::Advance(const char ch)
{
	Character++;
	if (ch == '\t')
	{
		Column += 4;
	}
	else
	{
		Column++;
	}

	if (ch == '\n')
	{
		Line++;
		Column = 0;
		Character = 0;
	}
}

