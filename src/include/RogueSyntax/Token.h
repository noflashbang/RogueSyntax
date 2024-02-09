#pragma once

#include "StandardLib.h"

namespace Token
{
	enum class TokenType:unsigned int
	{
		RS_ILLEGAL = 0,
		RS_EOF,

		// Identifiers + literals
		RS_IDENT,
		RS_INT,

		// Operators
		RS_ASSIGN,
		RS_PLUS,

		// Delimiters
		RS_COMMA,
		RS_SEMICOLON,

		RS_LPAREN,
		RS_RPAREN,
		RS_LBRACE,
		RS_RBRACE,

		// Keywords
		RS_FUNCTION,
		RS_LET
	};

	const std::map<TokenType, std::string> TokenNames = {
		{TokenType::RS_ILLEGAL, "ILLEGAL"},
		{TokenType::RS_EOF, "EOF"},

		{TokenType::RS_IDENT, "IDENT"},
		{TokenType::RS_INT, "INT"},

		{TokenType::RS_ASSIGN, "="},
		{TokenType::RS_PLUS, "+"},

		{TokenType::RS_COMMA, ","},
		{TokenType::RS_SEMICOLON, ";"},

		{TokenType::RS_LPAREN, "("},
		{TokenType::RS_RPAREN, ")"},
		{TokenType::RS_LBRACE, "{"},
		{TokenType::RS_RBRACE, "}"},

		{TokenType::RS_FUNCTION, "FUNCTION"},
		{TokenType::RS_LET, "LET"}
	};

	const std::map<std::string, TokenType> Keywords = {
		{"fn", TokenType::RS_FUNCTION},
		{"let", TokenType::RS_LET}
	};

	

	struct TokenLocation
	{
		unsigned int Line;
		unsigned int Column;
		std::string Filename;
	};

	struct Token
	{
		Token() : Type(TokenType::RS_ILLEGAL), Literal("") {}
		Token(TokenType type, char literal) : Type(type), Literal(std::string(1, literal)) {}
		Token(TokenType type, std::string literal) : Type(type), Literal(literal) {}

		constexpr std::string TypeName() const
		{
			return TokenNames.at(Type);
		}

		static TokenType LookupIdent(const std::string& ident)
		{
			auto it = Keywords.find(ident);
			if (it != Keywords.end())
			{
				return it->second;
			}
			return TokenType::RS_IDENT;
		}

		TokenType Type;
		std::string Literal;
		TokenLocation Location;
	};

} // namespace Token