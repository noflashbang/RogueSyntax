
#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Lexer Tests")
{
	
SECTION("Test NextToken")
	{
		Lexer lexer("let x = 5;");
		
		Token nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_LET);
		REQUIRE(nt.Literal == "let");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IDENT);
		REQUIRE(nt.Literal == "x");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_ASSIGN);
		REQUIRE(nt.Literal == "=");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_INT);
		REQUIRE(nt.Literal == "5");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_SEMICOLON);
		REQUIRE(nt.Literal == ";");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_EOF);
		REQUIRE(nt.Literal == "");
	}

	SECTION("Test NextToken All")
	{
		const std::string testCase = 
R"(let five = 5;
let ten = 10;
let add = fn(x, y) {
	x + y;
};
let result = add(five, ten);
)";
			
		Lexer lexer(testCase);
		Token nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_LET);
		REQUIRE(nt.Literal == "let");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IDENT);
		REQUIRE(nt.Literal == "five");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_ASSIGN);
		REQUIRE(nt.Literal == "=");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_INT);
		REQUIRE(nt.Literal == "5");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_SEMICOLON);
		REQUIRE(nt.Literal == ";");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_LET);
		REQUIRE(nt.Literal == "let");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IDENT);
		REQUIRE(nt.Literal == "ten");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_ASSIGN);
		REQUIRE(nt.Literal == "=");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_INT);
		REQUIRE(nt.Literal == "10");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_SEMICOLON);
		REQUIRE(nt.Literal == ";");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_LET);
		REQUIRE(nt.Literal == "let");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IDENT);
		REQUIRE(nt.Literal == "add");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_ASSIGN);
		REQUIRE(nt.Literal == "=");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_FUNCTION);
		REQUIRE(nt.Literal == "fn");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_LPAREN);
		REQUIRE(nt.Literal == "(");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IDENT);
		REQUIRE(nt.Literal == "x");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_COMMA);
		REQUIRE(nt.Literal == ",");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IDENT);
		REQUIRE(nt.Literal == "y");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_RPAREN);
		REQUIRE(nt.Literal == ")");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_LBRACE);
		REQUIRE(nt.Literal == "{");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IDENT);
		REQUIRE(nt.Literal == "x");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_PLUS);
		REQUIRE(nt.Literal == "+");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IDENT);
		REQUIRE(nt.Literal == "y");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_SEMICOLON);
		REQUIRE(nt.Literal == ";");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_RBRACE);
		REQUIRE(nt.Literal == "}");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_SEMICOLON);
		REQUIRE(nt.Literal == ";");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_LET);
		REQUIRE(nt.Literal == "let");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IDENT);
		REQUIRE(nt.Literal == "result");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_ASSIGN);
		REQUIRE(nt.Literal == "=");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IDENT);
		REQUIRE(nt.Literal == "add");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_LPAREN);
		REQUIRE(nt.Literal == "(");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IDENT);
		REQUIRE(nt.Literal == "five");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_COMMA);
		REQUIRE(nt.Literal == ",");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IDENT);
		REQUIRE(nt.Literal == "ten");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_RPAREN);
		REQUIRE(nt.Literal == ")");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_SEMICOLON);
		REQUIRE(nt.Literal == ";");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_EOF);
		REQUIRE(nt.Literal == "");
	}
}