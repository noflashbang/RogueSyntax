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

const std::map<std::string, TokenType> TokenType::KEYWORD_TOKEN_MAP = {
	{"fn",  TokenType::TOKEN_FUNCTION},
	{"let", TokenType::TOKEN_LET}
};


