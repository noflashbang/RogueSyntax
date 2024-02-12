
#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Parser Tests")
{
	SECTION("Test Identifier")
	{
		std::string input = "foobar;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program.Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program.Statements[0].get());
		REQUIRE(expressionStatement != nullptr);
		auto ident = dynamic_cast<Identifier*>(expressionStatement->Expression.get());
		REQUIRE(ident != nullptr);
		REQUIRE(ident->Value == "foobar");
		REQUIRE(ident->TokenLiteral() == "foobar");
	}

	SECTION("Test Integer Literal")
	{
		std::string input = "5;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program.Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program.Statements[0].get());
		REQUIRE(expressionStatement != nullptr);
		auto integer = dynamic_cast<IntegerLiteral*>(expressionStatement->Expression.get());
		REQUIRE(integer != nullptr);
		REQUIRE(integer->Value == 5);
		REQUIRE(integer->TokenLiteral() == "5");
	}

	SECTION("Test Prefix Expression BANG")
	{
		std::string input = "!5;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program.Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program.Statements[0].get());
		REQUIRE(expressionStatement != nullptr);
		auto prefix = dynamic_cast<PrefixExpression*>(expressionStatement->Expression.get());
		REQUIRE(prefix != nullptr);
		REQUIRE(prefix->Operator == "!");
		REQUIRE(prefix->Right->TokenLiteral() == "5");
	}

	SECTION("Test Prefix Expression MINUS")
	{
		std::string input = "-15;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program.Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program.Statements[0].get());
		REQUIRE(expressionStatement != nullptr);
		auto prefix = dynamic_cast<PrefixExpression*>(expressionStatement->Expression.get());
		REQUIRE(prefix != nullptr);
		REQUIRE(prefix->Operator == "-");
		REQUIRE(prefix->Right->TokenLiteral() == "15");
	}

	SECTION("Test Infix Expression")
	{
		std::string input = "5 + 5;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program.Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program.Statements[0].get());
		REQUIRE(expressionStatement != nullptr);
		auto infix = dynamic_cast<InfixExpression*>(expressionStatement->Expression.get());
		REQUIRE(infix != nullptr);
		REQUIRE(infix->Operator == "+");
		REQUIRE(infix->Left->TokenLiteral() == "5");
		REQUIRE(infix->Right->TokenLiteral() == "5");
	}

SECTION("Test Operator Precedence Parsing")
	{
		struct Test
		{
			std::string input;
			std::string expected;
		};

		std::vector<Test> tests = {
			{"-a * b", "((-a) * b)"},
			{"!-a", "(!(-a))"},
			{"a + b + c", "((a + b) + c)"},
			{"a + b - c", "((a + b) - c)"},
			{"a * b * c", "((a * b) * c)"},
			{"a * b / c", "((a * b) / c)"},
			{"a + b / c", "(a + (b / c))"},
			{"a + b * c + d / e - f", "(((a + (b * c)) + (d / e)) - f)"},
			{"3 + 4; -5 * 5", "(3 + 4)((-5) * 5)"},
			{"5 > 4 == 3 < 4", "((5 > 4) == (3 < 4))"},
			{"5 < 4 != 3 > 4", "((5 < 4) != (3 > 4))"},
			{"3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"},
		};

		for (auto& test : tests)
		{
			Lexer lexer(test.input);
			Parser parser(lexer);

			auto program = parser.ParseProgram();
			auto errors = parser.Errors();
			REQUIRE(errors.size() == 0);
			REQUIRE(program.ToString() == test.expected);
		}
	}

	SECTION("Test Let Statement")
	{
		std::string input = "let x = 5;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program.Statements.size() == 1);
		auto letStatement = dynamic_cast<LetStatement*>(program.Statements[0].get());
		REQUIRE(letStatement != nullptr);
		REQUIRE(letStatement->TokenLiteral() == "let");
		REQUIRE(letStatement->Name->Value == "x");
		REQUIRE(letStatement->Name->TokenLiteral() == "x");
		REQUIRE(letStatement->Value->TokenLiteral() == "5");
	}

	SECTION("Test Errors")
	{
		std::string input = "let x 5;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() != 0);
	}

	SECTION("Test Return Statement")
	{
		std::string input = "return 5;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program.Statements.size() == 1);
		auto returnStatement = dynamic_cast<ReturnStatement*>(program.Statements[0].get());
		REQUIRE(returnStatement != nullptr);
		REQUIRE(returnStatement->TokenLiteral() == "return");
		REQUIRE(returnStatement->ReturnValue->TokenLiteral() == "5");
	}
}