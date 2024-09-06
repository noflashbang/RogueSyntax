
#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Lexer Tests")
{
	SECTION("Test Location Information")
	{
		Lexer lexer("let x = 5;");

		UNSCOPED_INFO("let x = 5;");

		Token nt = lexer.NextToken();
		REQUIRE(nt.Location.Line == 1);
		REQUIRE(nt.Location.Column == 1);
		REQUIRE(nt.Location.Character == 1);

		nt = lexer.NextToken();
		REQUIRE(nt.Location.Line == 1);
		REQUIRE(nt.Location.Column == 5);
		REQUIRE(nt.Location.Character == 5);

		nt = lexer.NextToken();
		REQUIRE(nt.Location.Line == 1);
		REQUIRE(nt.Location.Column == 7);
		REQUIRE(nt.Location.Character == 7);

		nt = lexer.NextToken();
		REQUIRE(nt.Location.Line == 1);
		REQUIRE(nt.Location.Column == 9);
		REQUIRE(nt.Location.Character == 9);

		nt = lexer.NextToken();
		REQUIRE(nt.Location.Line == 1);
		REQUIRE(nt.Location.Column == 10);
		REQUIRE(nt.Location.Character == 10);
	}

	SECTION("Test Location Information")
	{
		Lexer lexer("\t\t\tlet\n \t\tx = 5;");

		UNSCOPED_INFO("\t\t\tlet\n \t\tx = 5;");

		Token nt = lexer.NextToken();
		REQUIRE(nt.Location.Line == 1);
		REQUIRE(nt.Location.Column == 13);
		REQUIRE(nt.Location.Character == 4);

		nt = lexer.NextToken();
		REQUIRE(nt.Location.Line == 2);
		REQUIRE(nt.Location.Column == 10);
		REQUIRE(nt.Location.Character == 4);

		nt = lexer.NextToken();
		REQUIRE(nt.Location.Line == 2);
		REQUIRE(nt.Location.Column == 12);
		REQUIRE(nt.Location.Character == 6);

		nt = lexer.NextToken();
		REQUIRE(nt.Location.Line == 2);
		REQUIRE(nt.Location.Column == 14);
		REQUIRE(nt.Location.Character == 8);

		nt = lexer.NextToken();
		REQUIRE(nt.Location.Line == 2);
		REQUIRE(nt.Location.Column == 15);
		REQUIRE(nt.Location.Character == 9);
	}

	SECTION("Test NextToken")
	{
		Lexer lexer("let x = 5;");
		
		UNSCOPED_INFO("let x = 5;");

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
			
		UNSCOPED_INFO(testCase);

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

	SECTION("Test NextToken All")
	{
		const std::string testCase = "let return if else true false fn {}()!= ! == = + - * / > < ; , 234 x somevar ^";

		Lexer lexer(testCase);

		UNSCOPED_INFO(testCase);

		Token nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_LET);
		REQUIRE(nt.Literal == "let");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_RETURN);
		REQUIRE(nt.Literal == "return");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IF);
		REQUIRE(nt.Literal == "if");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_ELSE);
		REQUIRE(nt.Literal == "else");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_TRUE);
		REQUIRE(nt.Literal == "true");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_FALSE);
		REQUIRE(nt.Literal == "false");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_FUNCTION);
		REQUIRE(nt.Literal == "fn");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_LBRACE);
		REQUIRE(nt.Literal == "{");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_RBRACE);
		REQUIRE(nt.Literal == "}");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_LPAREN);
		REQUIRE(nt.Literal == "(");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_RPAREN);
		REQUIRE(nt.Literal == ")");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_NOT_EQ);
		REQUIRE(nt.Literal == "!=");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_BANG);
		REQUIRE(nt.Literal == "!");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_EQ);
		REQUIRE(nt.Literal == "==");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_ASSIGN);
		REQUIRE(nt.Literal == "=");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_PLUS);
		REQUIRE(nt.Literal == "+");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_MINUS);
		REQUIRE(nt.Literal == "-");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_ASTERISK);
		REQUIRE(nt.Literal == "*");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_SLASH);
		REQUIRE(nt.Literal == "/");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_GT);
		REQUIRE(nt.Literal == ">");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_LT);
		REQUIRE(nt.Literal == "<");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_SEMICOLON);
		REQUIRE(nt.Literal == ";");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_COMMA);
		REQUIRE(nt.Literal == ",");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_INT);
		REQUIRE(nt.Literal == "234");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IDENT);
		REQUIRE(nt.Literal == "x");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_IDENT);
		REQUIRE(nt.Literal == "somevar");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_ILLEGAL);
		REQUIRE(nt.Literal == "^");

		nt = lexer.NextToken();
		REQUIRE(nt.Type == TokenType::TOKEN_EOF);
		REQUIRE(nt.Literal == "");

	}
}