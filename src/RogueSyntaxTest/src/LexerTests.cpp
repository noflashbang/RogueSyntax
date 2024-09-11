
#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>


namespace Catch {
	template<>
	struct StringMaker<TokenLocation> {
		static std::string convert(TokenLocation const& value) {
			return std::format("TokenLocation:{},{},{}", value.Line, value.Character, value.Column);
		}
	};
}

namespace Catch {
	template<>
	struct StringMaker<Token> {
		static std::string convert(Token const& value) {
			return std::format("Token:{},{}", value.Type.Name, value.Literal);
		}
	};
}

static bool operator==(const Token& lhs, const Token& rhs)
{
	return lhs.Type == rhs.Type && lhs.Literal == rhs.Literal;
};

static bool operator==(const TokenLocation& lhs, const TokenLocation& rhs)
{
	return lhs.Line == rhs.Line && lhs.Column == rhs.Column && lhs.Character == rhs.Character;
};


TEST_CASE("Test Location Information 1")
{
	auto test = std::tuple<std::string, std::vector<TokenLocation>>{ "let x = 5;", { TokenLocation{1,1,1}, TokenLocation{ 1, 5, 5 }, TokenLocation{ 1, 7, 7 }, TokenLocation{ 1, 9, 9 }, TokenLocation{ 1, 10, 10 } } };
	
	auto [input, expectedLocations] = test;
	Lexer lexer(input);

	INFO(input);

	for (const auto& expectedLocation : expectedLocations)
	{
		Token nt = lexer.NextToken();
		CHECK(nt.Location == expectedLocation);
	}
}

TEST_CASE("Test Location Information 2")
{
	auto test = std::tuple<std::string, std::vector<TokenLocation>>{ "\t\t\tlet\n \t\tx = 5;", { TokenLocation{1,4,13}, TokenLocation{ 2, 4, 10 }, TokenLocation{ 2, 6, 12 }, TokenLocation{ 2, 8, 14}, TokenLocation{ 2, 9, 15 } } };

	auto [input, expectedLocations] = test;
	Lexer lexer(input);

	INFO(input);

	for (const auto& expectedLocation : expectedLocations)
	{
		Token nt = lexer.NextToken();
		CHECK(nt.Location == expectedLocation);
	}
}

TEST_CASE("Test NextToken 1")
{
	auto test = std::tuple<std::string, std::vector<Token>>{ "let x = 5;", { {TokenType::TOKEN_LET, "let"}, {TokenType::TOKEN_IDENT, "x"}, {TokenType::TOKEN_ASSIGN, "="}, {TokenType::TOKEN_INT, "5"}, {TokenType::TOKEN_SEMICOLON, ";"}, {TokenType::TOKEN_EOF, ""} } };

	auto [input, expectedTokens] = test;
	Lexer lexer(input);

	INFO(input);

	for (const auto& expected : expectedTokens)
	{
		Token nt = lexer.NextToken();
		CHECK(nt == expected);
	}
}

TEST_CASE("Test NextToken 2")
{
	auto test = std::tuple<std::string, std::vector<Token>>{ "let five = 5; let ten = 10; let add = fn(x, y) { x + y; }; let result = add(five, ten);", { {TokenType::TOKEN_LET, "let"}, {TokenType::TOKEN_IDENT, "five"}, {TokenType::TOKEN_ASSIGN, "="}, {TokenType::TOKEN_INT, "5"}, {TokenType::TOKEN_SEMICOLON, ";"}, {TokenType::TOKEN_LET, "let"}, {TokenType::TOKEN_IDENT, "ten"}, {TokenType::TOKEN_ASSIGN, "="}, {TokenType::TOKEN_INT, "10"}, {TokenType::TOKEN_SEMICOLON, ";"}, {TokenType::TOKEN_LET, "let"}, {TokenType::TOKEN_IDENT, "add"}, {TokenType::TOKEN_ASSIGN, "="}, {TokenType::TOKEN_FUNCTION , "fn"}, {TokenType::TOKEN_LPAREN, "("}, {TokenType::TOKEN_IDENT, "x"}, {TokenType::TOKEN_COMMA, ","}, {TokenType::TOKEN_IDENT , "y"}, {TokenType::TOKEN_RPAREN, ")"}, {TokenType::TOKEN_LBRACE, "{"}, {TokenType::TOKEN_IDENT, "x"}, {TokenType::TOKEN_PLUS, "+"}, {TokenType::TOKEN_IDENT , "y"}, {TokenType::TOKEN_SEMICOLON, ";"}, {TokenType::TOKEN_RBRACE, "}"}, {TokenType::TOKEN_SEMICOLON, ";"}, {TokenType::TOKEN_LET, "let"}, {TokenType::TOKEN_IDENT, "result"}, {TokenType::TOKEN_ASSIGN, "="}, {TokenType::TOKEN_IDENT, "add"}, {TokenType::TOKEN_LPAREN, "("}, {TokenType::TOKEN_IDENT , "five"}, {TokenType::TOKEN_COMMA, ","}, {TokenType::TOKEN_IDENT, "ten"}, {TokenType::TOKEN_RPAREN, ")"}, {TokenType::TOKEN_SEMICOLON, ";" }, {TokenType::TOKEN_EOF, ""} } };

	auto [input, expectedTokens] = test;
	Lexer lexer(input);

	INFO(input);

	for (const auto& expected : expectedTokens)
	{
		Token nt = lexer.NextToken();
		CHECK(nt == expected);
	}
}

TEST_CASE("Test NextToken 3")
{

	auto test = std::tuple < std::string, std::vector < Token >>{ "let return if else true false fn {}()!= ! == = + - * / > < ; , 234 x somevar ^ while break continue for 23d 2.3 \"some string\" [ ] :",
		{
			{TokenType::TOKEN_LET, "let"},
			{TokenType::TOKEN_RETURN, "return"},
			{TokenType::TOKEN_IF, "if"},
			{TokenType::TOKEN_ELSE, "else"},
			{TokenType::TOKEN_TRUE, "true"},
			{TokenType::TOKEN_FALSE, "false"},
			{TokenType::TOKEN_FUNCTION, "fn"},
			{TokenType::TOKEN_LBRACE, "{"},
			{TokenType::TOKEN_RBRACE, "}"},
			{TokenType::TOKEN_LPAREN, "("},
			{TokenType::TOKEN_RPAREN, ")"},
			{TokenType::TOKEN_NOT_EQ, "!="},
			{TokenType::TOKEN_BANG, "!"},
			{TokenType::TOKEN_EQ, "=="},
			{TokenType::TOKEN_ASSIGN, "="},
			{TokenType::TOKEN_PLUS, "+"},
			{TokenType::TOKEN_MINUS, "-"},
			{TokenType::TOKEN_ASTERISK, "*"},
			{TokenType::TOKEN_SLASH, "/"},
			{TokenType::TOKEN_GT, ">"},
			{TokenType::TOKEN_LT, "<"},
			{TokenType::TOKEN_SEMICOLON, ";"},
			{TokenType::TOKEN_COMMA, ","},
			{TokenType::TOKEN_INT, "234"},
			{TokenType::TOKEN_IDENT, "x"},
			{TokenType::TOKEN_IDENT , "somevar"},
			{TokenType::TOKEN_ILLEGAL, "^"},
			{TokenType::TOKEN_WHILE, "while"},
			{TokenType::TOKEN_BREAK, "break"},
			{TokenType::TOKEN_CONTINUE, "continue"},
			{TokenType::TOKEN_FOR, "for"},
			{TokenType::TOKEN_DECIMAL, "23d"},
			{TokenType::TOKEN_DECIMAL, "2.3"},
			{TokenType::TOKEN_STRING, "\"some string\""},
			{TokenType::TOKEN_LBRACKET, "["},
			{TokenType::TOKEN_RBRACKET, "]"},
			{TokenType::TOKEN_COLON, ":"},
			{TokenType::TOKEN_EOF, ""}
		} };

	auto [input, expectedTokens] = test;
	Lexer lexer(input);

	INFO(input);

	for (const auto& expected : expectedTokens)
	{
		Token nt = lexer.NextToken();
		CHECK(nt == expected);
	}
}