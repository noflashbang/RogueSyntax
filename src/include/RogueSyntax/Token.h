#pragma once

#include "StandardLib.h"


struct TokenType
{
	unsigned int Type;
	std::string Name;

	constexpr bool operator==(const TokenType& other) const
	{
		return Type == other.Type;
	}

	constexpr bool operator!=(const TokenType& other) const
	{
		return Type != other.Type;
	}

	constexpr bool operator<(const TokenType& other) const
	{
		return Type < other.Type;
	}

	constexpr bool operator>(const TokenType& other) const
	{
		return Type > other.Type;
	}

	constexpr bool operator<=(const TokenType& other) const
	{
		return Type <= other.Type;
	}

	constexpr bool operator>=(const TokenType& other) const
	{
		return Type >= other.Type;
	}

	explicit constexpr operator unsigned int() const
	{
		return Type;
	}

	explicit constexpr operator std::string() const
	{
		return Name;
	}

	std::string ToString() const
	{
		return Name;
	}

	//================================================================================
	// Special tokens
	static const TokenType TOKEN_ILLEGAL;
	static const TokenType TOKEN_EOF;

	// Identifiers + literals
	static const TokenType TOKEN_IDENT;
	static const TokenType TOKEN_INT;

	// Operators
	static const TokenType TOKEN_ASSIGN;
	static const TokenType TOKEN_PLUS;

	// Delimiters
	static const TokenType TOKEN_COMMA;
	static const TokenType TOKEN_SEMICOLON;
	static const TokenType TOKEN_LPAREN;
	static const TokenType TOKEN_RPAREN;
	static const TokenType TOKEN_LBRACE;
	static const TokenType TOKEN_RBRACE;

	// Keywords
	static const TokenType TOKEN_FUNCTION;
	static const TokenType TOKEN_LET;

	static TokenType LookupIdent(const std::string& ident)
	{
		auto it = KEYWORD_TOKEN_MAP.find(ident);
		if (it != KEYWORD_TOKEN_MAP.end())
		{
			return it->second;
		}
		return TokenType::TOKEN_IDENT;
	}

private:

	static const std::map<std::string, TokenType> KEYWORD_TOKEN_MAP;
};

struct TokenLocation
{
	unsigned int Line;
	unsigned int Character;
	unsigned int Column;
	std::string Filename;
};

struct Token
{
	TokenType Type;
	std::string Literal;
	TokenLocation Location;

	//methods for construction - helpers
	static Token New() { return  { TokenType::TOKEN_ILLEGAL, "" }; };
	static Token New(const TokenType type, const char literal) { return  { type, std::string(1, literal) }; };
	static Token New(const TokenType type, const std::string& literal) { return  { type, literal }; };

	std::string TypeName() const
	{
		return Type.ToString();
	}
};

