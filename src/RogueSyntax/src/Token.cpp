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
const TokenType TokenType::TOKEN_MINUS     = { 6,  "-" };
const TokenType TokenType::TOKEN_BANG      = { 7,  "!" };
const TokenType TokenType::TOKEN_ASTERISK  = { 8,  "*" };
const TokenType TokenType::TOKEN_SLASH     = { 9,  "/" };
const TokenType TokenType::TOKEN_LT        = { 10, "<" };
const TokenType TokenType::TOKEN_GT        = { 11, ">" };
const TokenType TokenType::TOKEN_EQ        = { 12, "==" };
const TokenType TokenType::TOKEN_NOT_EQ    = { 13, "!=" };

// Delimiters							   		   
const TokenType TokenType::TOKEN_COMMA     = { 6,  "," };
const TokenType TokenType::TOKEN_SEMICOLON = { 7,  ";" };
												   
const TokenType TokenType::TOKEN_LPAREN    = { 8,  "(" };
const TokenType TokenType::TOKEN_RPAREN    = { 9,  ")" };
const TokenType TokenType::TOKEN_LBRACE    = { 10, "{" };
const TokenType TokenType::TOKEN_RBRACE    = { 11, "}" };

// Keywords
const TokenType TokenType::TOKEN_FUNCTION  = { 12, "FUNCTION" };
const TokenType TokenType::TOKEN_LET       = { 13, "LET" };
const TokenType TokenType::TOKEN_TRUE      = { 14, "TRUE" };
const TokenType TokenType::TOKEN_FALSE     = { 15, "FALSE" };
const TokenType TokenType::TOKEN_IF        = { 16, "IF" };
const TokenType TokenType::TOKEN_ELSE      = { 17, "ELSE" };
const TokenType TokenType::TOKEN_RETURN    = { 18, "RETURN" };

const std::map<std::string, TokenType> TokenType::KEYWORD_TOKEN_MAP = {
	{"fn",  TokenType::TOKEN_FUNCTION},
	{"let", TokenType::TOKEN_LET},
	{"true", TokenType::TOKEN_TRUE},
	{"false", TokenType::TOKEN_FALSE},
	{"if", TokenType::TOKEN_IF},
	{"else", TokenType::TOKEN_ELSE},
	{"return", TokenType::TOKEN_RETURN}
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

