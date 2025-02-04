#include "pch.h"

// Initialize the static integer
unsigned int TokenType::NextTokenNumber = 0;

// Special tokens
const TokenType TokenType::TOKEN_ILLEGAL       = { TokenType::NextTokenNumber++,  "ILLEGAL" };
const TokenType TokenType::TOKEN_EOF           = { TokenType::NextTokenNumber++,  "EOF" };
										   		   
// Identifiers + literals				   		   
const TokenType TokenType::TOKEN_IDENT         = { TokenType::NextTokenNumber++, "IDENT" };
const TokenType TokenType::TOKEN_INT           = { TokenType::NextTokenNumber++, "INT" };
const TokenType TokenType::TOKEN_STRING        = { TokenType::NextTokenNumber++, "STRING" };
const TokenType TokenType::TOKEN_DECIMAL       = { TokenType::NextTokenNumber++, "DECIMAL" };
										   		   
// Operators							   		   
const TokenType TokenType::TOKEN_ASSIGN        = { TokenType::NextTokenNumber++, "=" };
const TokenType TokenType::TOKEN_PLUS          = { TokenType::NextTokenNumber++, "+" };
const TokenType TokenType::TOKEN_INCREMENT     = { TokenType::NextTokenNumber++, "++" };
const TokenType TokenType::TOKEN_MINUS         = { TokenType::NextTokenNumber++, "-" };
const TokenType TokenType::TOKEN_DECREMENT     = { TokenType::NextTokenNumber++, "--" };
const TokenType TokenType::TOKEN_BANG          = { TokenType::NextTokenNumber++, "!" };
const TokenType TokenType::TOKEN_ASTERISK      = { TokenType::NextTokenNumber++, "*" };
const TokenType TokenType::TOKEN_SLASH         = { TokenType::NextTokenNumber++, "/" };
const TokenType TokenType::TOKEN_MODULO        = { TokenType::NextTokenNumber++, "%" };
const TokenType TokenType::TOKEN_GT_EQ         = { TokenType::NextTokenNumber++, ">=" };
const TokenType TokenType::TOKEN_LT_EQ         = { TokenType::NextTokenNumber++, "<=" };
const TokenType TokenType::TOKEN_LT            = { TokenType::NextTokenNumber++, "<" };
const TokenType TokenType::TOKEN_GT            = { TokenType::NextTokenNumber++, ">" };
const TokenType TokenType::TOKEN_EQ            = { TokenType::NextTokenNumber++, "==" };
const TokenType TokenType::TOKEN_NOT_EQ        = { TokenType::NextTokenNumber++, "!=" };

const TokenType TokenType::TOKEN_AND           = { TokenType::NextTokenNumber++, "&&" };
const TokenType TokenType::TOKEN_OR            = { TokenType::NextTokenNumber++, "||" };
const TokenType TokenType::TOKEN_BITWISE_AND   = { TokenType::NextTokenNumber++, "&" };
const TokenType TokenType::TOKEN_BITWISE_OR    = { TokenType::NextTokenNumber++, "|" };
const TokenType TokenType::TOKEN_BITWISE_XOR   = { TokenType::NextTokenNumber++, "^" };
const TokenType TokenType::TOKEN_BITWISE_NOT   = { TokenType::NextTokenNumber++, "~" };

const TokenType TokenType::TOKEN_SHIFT_LEFT    = { TokenType::NextTokenNumber++, "<<" };
const TokenType TokenType::TOKEN_SHIFT_RIGHT   = { TokenType::NextTokenNumber++, ">>" };

const TokenType TokenType::TOKEN_PLUS_ASSIGN = { TokenType::NextTokenNumber++, "+=" };
const TokenType TokenType::TOKEN_MINUS_ASSIGN = { TokenType::NextTokenNumber++, "-=" };
const TokenType TokenType::TOKEN_ASTERISK_ASSIGN = { TokenType::NextTokenNumber++, "*=" };
const TokenType TokenType::TOKEN_SLASH_ASSIGN = { TokenType::NextTokenNumber++, "/=" };
const TokenType TokenType::TOKEN_MODULO_ASSIGN = { TokenType::NextTokenNumber++, "%=" };

// Delimiters							   		   
const TokenType TokenType::TOKEN_COMMA         = { TokenType::NextTokenNumber++, "," };
const TokenType TokenType::TOKEN_SEMICOLON     = { TokenType::NextTokenNumber++, ";" };
const TokenType TokenType::TOKEN_COLON         = { TokenType::NextTokenNumber++, ":" };
const TokenType TokenType::TOKEN_LPAREN        = { TokenType::NextTokenNumber++, "(" };
const TokenType TokenType::TOKEN_RPAREN        = { TokenType::NextTokenNumber++, ")" };
const TokenType TokenType::TOKEN_LBRACE        = { TokenType::NextTokenNumber++, "{" };
const TokenType TokenType::TOKEN_RBRACE        = { TokenType::NextTokenNumber++, "}" };
const TokenType TokenType::TOKEN_LBRACKET      = { TokenType::NextTokenNumber++, "[" };
const TokenType TokenType::TOKEN_RBRACKET      = { TokenType::NextTokenNumber++, "]" };

// Keywords
const TokenType TokenType::TOKEN_NULL          = { TokenType::NextTokenNumber++, "NULL" };
const TokenType TokenType::TOKEN_FUNCTION      = { TokenType::NextTokenNumber++, "FUNCTION" };
const TokenType TokenType::TOKEN_LET           = { TokenType::NextTokenNumber++, "LET" };
const TokenType TokenType::TOKEN_TRUE          = { TokenType::NextTokenNumber++, "TRUE" };
const TokenType TokenType::TOKEN_FALSE         = { TokenType::NextTokenNumber++, "FALSE" };
const TokenType TokenType::TOKEN_IF            = { TokenType::NextTokenNumber++, "IF" };
const TokenType TokenType::TOKEN_ELSE          = { TokenType::NextTokenNumber++, "ELSE" };
const TokenType TokenType::TOKEN_RETURN        = { TokenType::NextTokenNumber++, "RETURN" };
const TokenType TokenType::TOKEN_WHILE         = { TokenType::NextTokenNumber++, "WHILE" };
const TokenType TokenType::TOKEN_BREAK         = { TokenType::NextTokenNumber++, "BREAK" };
const TokenType TokenType::TOKEN_CONTINUE      = { TokenType::NextTokenNumber++, "CONTINUE" };
const TokenType TokenType::TOKEN_FOR           = { TokenType::NextTokenNumber++, "FOR" };

//comments
const TokenType TokenType::TOKEN_COMMENT       = { TokenType::NextTokenNumber++, "//" };

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
	{"null", TokenType::TOKEN_NULL}
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

