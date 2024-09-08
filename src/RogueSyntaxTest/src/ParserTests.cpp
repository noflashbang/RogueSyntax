
#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>


bool TestIdentifier(const IExpression* expression, const std::string& expected)
{
	auto ident = dynamic_cast<const Identifier*>(expression);
	if (ident == nullptr)
	{
		throw std::invalid_argument(std::format("Bad expression -> Expected Identifier GOT {}", expression->ToString()));
		return false;
	}

	if (ident->Value != expected)
	{
		throw std::invalid_argument(std::format("Bad Value -> Expected {} GOT {}", expected, ident->Value));
		return false;
	}

	if(ident->TokenLiteral() != expected)
	{
		throw std::invalid_argument(std::format("Bad Literal -> Expected {} GOT {}", expected, ident->Value));
		return false;
	}

	return true;
}

bool TestIntegerLiteral(const IExpression* expression, int expected)
{
	auto integer = dynamic_cast<const IntegerLiteral*>(expression);
	if (integer == nullptr)
	{
		throw std::invalid_argument(std::format("Bad expression -> Expected IntegerLiteral GOT {}", expression->ToString()));
		return false;
	}

	if (integer->Value != expected)
	{
		throw std::invalid_argument(std::format("Bad Value -> Expected {} GOT {}", expected, integer->Value));
		return false;
	}

	if (integer->TokenLiteral() != std::to_string(expected))
	{
		throw std::invalid_argument(std::format("Bad Literal -> Expected {} GOT {}", expected, integer->Value));
		return false;
	}

	return true;
}

bool TestBooleanLiteral(const IExpression* expression, bool expected)
{
	auto boolean = dynamic_cast<const BooleanLiteral*>(expression);
	if (boolean == nullptr)
	{
		throw std::invalid_argument(std::format("Bad expression -> Expected BooleanLiteral GOT {}", expression->ToString()));
		return false;
	}

	if (boolean->Value != expected)
	{
		throw std::invalid_argument(std::format("Bad Value -> Expected {} GOT {}", expected, boolean->Value));
		return false;
	}

	if(expected && boolean->TokenLiteral() != "true")
	{
		throw std::invalid_argument(std::format("Bad Literal -> Expected {} GOT {}", expected, boolean->Value));
		return false;
	}
	
	if(!expected && boolean->TokenLiteral() != "false")
	{
		throw std::invalid_argument(std::format("Bad Literal -> Expected {} GOT {}", expected, boolean->Value));
		return false;
	}

	return true;
}

bool TestLiteralExpression(const IExpression* expression, const std::string& expected)
{
	if (expression->Type() == TokenType::TOKEN_IDENT)
	{
		return TestIdentifier(expression, expected);
	}
	else if(expression->Type() == TokenType::TOKEN_INT)
	{
		return TestIntegerLiteral(expression, std::stoi(expected));
	}
	else if(expression->Type() == TokenType::TOKEN_TRUE || expression->Type() == TokenType::TOKEN_FALSE)
	{
		return TestBooleanLiteral(expression, expected == "true");
	}
	else
	{
		throw std::invalid_argument(std::format("Bad expression -> Expected Literal GOT {}", expression->ToString()));
		return false;
	}

	return true;
}

bool TestInfixExpression(const IExpression* expression, const std::string& left, const std::string& op, const std::string& right)
{
	auto infix = dynamic_cast<const InfixExpression*>(expression);
	if (infix == nullptr)
	{
		throw std::invalid_argument(std::format("Bad expression -> Expected InfixExpression GOT {}", expression->ToString()));
		return false;
	}

	if (!TestLiteralExpression(infix->Left.get(), left))
	{
		return false;
	}

	if (infix->Operator != op)
	{
		throw std::invalid_argument(std::format("Bad Operator -> Expected {} GOT {}", op, infix->Operator));
		return false;
	}

	if (!TestLiteralExpression(infix->Right.get(), right))
	{
		return false;
	}

	return true;
}

bool TestPrefixExpression(const IExpression* expression, const std::string& op, const std::string& right)
{
	auto prefix = dynamic_cast<const PrefixExpression*>(expression);
	if (prefix == nullptr)
	{
		throw std::invalid_argument(std::format("Bad expression -> Expected PrefixExpression GOT {}", expression->ToString()));
		return false;
	}

	if (prefix->Operator != op)
	{
		throw std::invalid_argument(std::format("Bad Operator -> Expected {} GOT {}", op, prefix->Operator));
		return false;
	}

	if (!TestLiteralExpression(prefix->Right.get(), right))
	{
		return false;
	}

	return true;
}

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

		REQUIRE(program->Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
		REQUIRE(expressionStatement != nullptr);

		REQUIRE(TestIdentifier(expressionStatement->Expression.get(), "foobar"));
	}

	SECTION("Test Integer Literal")
	{
		std::string input = "5;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
		REQUIRE(expressionStatement != nullptr);
		auto integer = dynamic_cast<IntegerLiteral*>(expressionStatement->Expression.get());

		REQUIRE(TestIntegerLiteral(expressionStatement->Expression.get(), 5));
	}

	SECTION("Test Boolean Literal")
	{
		std::string input = "true;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
		REQUIRE(expressionStatement != nullptr);

		REQUIRE(TestBooleanLiteral(expressionStatement->Expression.get(), true));
	}

	SECTION("Test Prefix Expression BANG")
	{
		std::string input = "!5;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
		REQUIRE(expressionStatement != nullptr);

		auto prefix = dynamic_cast<PrefixExpression*>(expressionStatement->Expression.get());
	}

	SECTION("Test Prefix Expression MINUS")
	{
		std::string input = "-15;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
		REQUIRE(expressionStatement != nullptr);

		REQUIRE(TestPrefixExpression(expressionStatement->Expression.get(), "-", "15"));
	}

	SECTION("Test postfix statements")
	{
		struct Test
		{
			std::string input;
			std::string expected;
		};

		std::vector<Test> tests = {
			{"x++;", "let x = (x + 1);"},
			{"t--;", "let t = (t - 1);"},
		};

		for (auto& test : tests)
		{
			Lexer lexer(test.input);
			Parser parser(lexer);

			auto program = parser.ParseProgram();
			auto errors = parser.Errors();
			REQUIRE(errors.size() == 0);

			REQUIRE(program->Statements.size() == 1);
			auto assignStatement = dynamic_cast<LetStatement*>(program->Statements[0].get());
			REQUIRE(assignStatement != nullptr);
			REQUIRE(assignStatement->ToString() == test.expected);
		}
	}

	SECTION("Test Infix Expression")
	{
		std::string input = "5 + 5;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
		REQUIRE(expressionStatement != nullptr);

		REQUIRE(TestInfixExpression(expressionStatement->Expression.get(), "5", "+", "5"));
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
			{"true", "true"},
			{"false", "false"},
			{"3 > 5 == false", "((3 > 5) == false)"},
			{"3 < 5 == true", "((3 < 5) == true)"},
			{"1 + (2 + 3) + 4", "((1 + (2 + 3)) + 4)"},
			{"(5 + 5) * 2", "((5 + 5) * 2)"},
			{"2 / (5 + 5)", "(2 / (5 + 5))"},
			{"-(5 + 5)", "(-(5 + 5))"},
			{"!(true == true)", "(!(true == true))"},
			{"a + add(b * c) + d", "((a + add((b * c))) + d)"},
			{"add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))", "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))"},
		};

		for (auto& test : tests)
		{
			Lexer lexer(test.input);
			Parser parser(lexer);

			auto program = parser.ParseProgram();
			auto errors = parser.Errors();
			if (errors.size() != 0)
			{
				for (auto& error : errors)
				{
					INFO(error);
				}
			}
			UNSCOPED_INFO(test.input);
			REQUIRE(errors.size() == 0);
			REQUIRE(program->ToString() == test.expected);
		}
	}

	SECTION("Test Parsing Infix Expressions")
	{
		struct Test
		{
			std::string input;
			std::string left;
			std::string op;
			std::string right;
		};

		std::vector<Test> tests = {
			{"5 + 5;", "5", "+", "5"},
			{"5 - 5;", "5", "-", "5"},
			{"5 * 5;", "5", "*", "5"},
			{"5 / 5;", "5", "/", "5"},
			{"5 > 5;", "5", ">", "5"},
			{"5 < 5;", "5", "<", "5"},
			{"5 == 5;", "5", "==", "5"},
			{"5 != 5;", "5", "!=", "5"},
		};

		for (auto& test : tests)
		{
			Lexer lexer(test.input);
			Parser parser(lexer);

			auto program = parser.ParseProgram();
			auto errors = parser.Errors();

			UNSCOPED_INFO(test.input);

			REQUIRE(errors.size() == 0);

			REQUIRE(program->Statements.size() == 1);
			auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
			REQUIRE(expressionStatement != nullptr);

			REQUIRE(TestInfixExpression(expressionStatement->Expression.get(), test.left, test.op, test.right));
		}
	}

	SECTION("Test Let Statement")
	{
		struct Test
		{
			std::string input;
			std::string expectedIdentifier;
			std::string expectedValue;
		};

		std::vector<Test> tests = {
			{"let x = 5;", "x", "5"},
			{"let y = true;", "y", "true"},
			{"let foobar = y;", "foobar", "y"},
		};

		for (auto& test : tests)
		{
			Lexer lexer(test.input);
			Parser parser(lexer);

			auto program = parser.ParseProgram();
			auto errors = parser.Errors();

			UNSCOPED_INFO(test.input);

			REQUIRE(errors.size() == 0);

			REQUIRE(program->Statements.size() == 1);
			auto letStatement = dynamic_cast<LetStatement*>(program->Statements[0].get());
			REQUIRE(letStatement != nullptr);

			REQUIRE(letStatement->TokenLiteral() == "let");
			REQUIRE(TestIdentifier(letStatement->Name.get(), test.expectedIdentifier));
			REQUIRE(TestLiteralExpression(letStatement->Value.get(), test.expectedValue));
		}
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
		struct Test
		{
			std::string input;
			std::string expectedValue;
		};

		std::vector<Test> tests = {
			{"return 5;", "5"},
			{"return true;", "true"},
			{"return foobar;", "foobar"},
		};

		for (auto& test : tests)
		{
			Lexer lexer(test.input);
			Parser parser(lexer);

			auto program = parser.ParseProgram();
			auto errors = parser.Errors();
			REQUIRE(errors.size() == 0);

			REQUIRE(program->Statements.size() == 1);
			auto returnStatement = dynamic_cast<ReturnStatement*>(program->Statements[0].get());
			REQUIRE(returnStatement != nullptr);

			REQUIRE(returnStatement->TokenLiteral() == "return");
			REQUIRE(TestLiteralExpression(returnStatement->ReturnValue.get(), test.expectedValue));
		}
	}

	SECTION("Test If Expression")
	{
		std::string input = "if (x < y) { x }";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);

		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
		REQUIRE(expressionStatement != nullptr);

		auto ifStatement = dynamic_cast<IfExpression*>(expressionStatement->Expression.get());		
		REQUIRE(ifStatement != nullptr);
		REQUIRE(ifStatement->TokenLiteral() == "if");
		
		REQUIRE(TestInfixExpression(ifStatement->Condition.get(), "x", "<", "y"));

		auto blockStatement = dynamic_cast<BlockStatement*>(ifStatement->Consequence.get());
		REQUIRE(blockStatement != nullptr);

		auto statement = dynamic_cast<ExpressionStatement*>(blockStatement->Statements[0].get());
		REQUIRE(statement != nullptr);
		REQUIRE(TestIdentifier(statement->Expression.get(), "x"));
	}

	SECTION("Test If Else Expression")
	{
		std::string input = "if (x < y) { x } else { y }";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
		REQUIRE(expressionStatement != nullptr);

		auto ifStatement = dynamic_cast<IfExpression*>(expressionStatement->Expression.get());
		REQUIRE(ifStatement != nullptr);
		REQUIRE(ifStatement->TokenLiteral() == "if");

		REQUIRE(TestInfixExpression(ifStatement->Condition.get(), "x", "<", "y"));

		auto consequence = dynamic_cast<BlockStatement*>(ifStatement->Consequence.get());
		REQUIRE(consequence != nullptr);

		auto statement = dynamic_cast<ExpressionStatement*>(consequence->Statements[0].get());
		REQUIRE(statement != nullptr);
		REQUIRE(TestIdentifier(statement->Expression.get(), "x"));
		


		auto alternative = dynamic_cast<BlockStatement*>(ifStatement->Alternative.get());
		REQUIRE(alternative != nullptr);

		statement = dynamic_cast<ExpressionStatement*>(alternative->Statements[0].get());
		REQUIRE(statement != nullptr);
		REQUIRE(TestIdentifier(statement->Expression.get(), "y"));

	}

	SECTION("Test Function Literal")
	{
		std::string input = "fn(x, y) { x + y; }";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
		REQUIRE(expressionStatement != nullptr);

		auto function = dynamic_cast<FunctionLiteral*>(expressionStatement->Expression.get());
		REQUIRE(function != nullptr);
		REQUIRE(function->TokenLiteral() == "fn");

		REQUIRE(function->Parameters.size() == 2);

		auto identifier = dynamic_cast<Identifier*>(function->Parameters[0].get());
		REQUIRE(identifier != nullptr);
		REQUIRE(identifier->Value == "x");

		identifier = dynamic_cast<Identifier*>(function->Parameters[1].get());
		REQUIRE(identifier != nullptr);
		REQUIRE(identifier->Value == "y");
		
		auto blockStatement = dynamic_cast<BlockStatement*>(function->Body.get());
		REQUIRE(blockStatement != nullptr);

		REQUIRE(blockStatement->Statements.size() == 1);
		auto statement = dynamic_cast<ExpressionStatement*>(blockStatement->Statements[0].get());
		REQUIRE(statement != nullptr);

		REQUIRE(TestInfixExpression(statement->Expression.get(), "x", "+", "y"));
	}

	SECTION("Test Call Expression")
	{
		std::string input = "add(1, 2 * 3, 4 + 5);";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
		REQUIRE(expressionStatement != nullptr);

		auto call = dynamic_cast<CallExpression*>(expressionStatement->Expression.get());
		REQUIRE(call != nullptr);
		REQUIRE(call->Function->TokenLiteral() == "add");

		REQUIRE(call->Arguments.size() == 3);

		REQUIRE(TestLiteralExpression(call->Arguments[0].get(), "1"));
		REQUIRE(TestInfixExpression(call->Arguments[1].get(), "2", "*", "3"));
		REQUIRE(TestInfixExpression(call->Arguments[2].get(), "4", "+", "5"));
	}

	SECTION("Test While expression")
	{
		std::string input = "while (x < y) { let y = y + 1; }";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
		REQUIRE(expressionStatement != nullptr);

		auto whileStatement = dynamic_cast<WhileExpression*>(expressionStatement->Expression.get());
		REQUIRE(whileStatement != nullptr);
		REQUIRE(whileStatement->TokenLiteral() == "while");

		REQUIRE(TestInfixExpression(whileStatement->Condition.get(), "x", "<", "y"));

		auto blockStatement = dynamic_cast<BlockStatement*>(whileStatement->Action.get());
		REQUIRE(blockStatement != nullptr);

		auto statement = dynamic_cast<LetStatement*>(blockStatement->Statements[0].get());
		REQUIRE(statement != nullptr);
		REQUIRE(statement->Value.get()->ToString() == "(y + 1)");
	}

	SECTION("Test Break Statement")
	{
		std::string input = "break;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto breakStatement = dynamic_cast<BreakStatement*>(program->Statements[0].get());
		REQUIRE(breakStatement != nullptr);
		REQUIRE(breakStatement->TokenLiteral() == "break");
	}

	SECTION("Test Continue Statement")
	{
		std::string input = "continue;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto continueStatement = dynamic_cast<ContinueStatement*>(program->Statements[0].get());
		REQUIRE(continueStatement != nullptr);
		REQUIRE(continueStatement->TokenLiteral() == "continue");
	}

	SECTION("Test assignemnt")
	{
		std::string input = "x = 5;";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto assignStatement = dynamic_cast<LetStatement*>(program->Statements[0].get());
		REQUIRE(assignStatement != nullptr);
		REQUIRE(assignStatement->Name->Value == "x");
		REQUIRE(assignStatement->Value->ToString() == "5");
	}

	SECTION("Test function assignemnt")
	{
		std::string input = "x = fn(x){ return x*x;};";
		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto assignStatement = dynamic_cast<LetStatement*>(program->Statements[0].get());
		REQUIRE(assignStatement != nullptr);
		REQUIRE(assignStatement->Name->Value == "x");
		REQUIRE(assignStatement->Value->ToString() == "fn(x){return (x * x);}");
	}

	SECTION("Test for parsing")
	{
		struct Test
		{
			std::string input;
			std::string expectedValue;
		};

		std::vector<Test> tests = {
			{"for(x=6; x<60;x++){ x; }"                       , "for (let x = 6; (x < 60); let x = (x + 1)) {x}"},
			{"for(let x=6; x<60;x = x+1){ x; }"               , "for (let x = 6; (x < 60); let x = (x + 1)) {x}"},
			{"for(x=6; x!=60;t(x)){ x; }"                     , "for (let x = 6; (x != 60); t(x)) {x}"},
			{"for (let x = 6; (x < 60); let x = (x + 1)) {x}" , "for (let x = 6; (x < 60); let x = (x + 1)) {x}"},
			{"for (let x = 6; (x < 60); let x = (x + 1)) {x}" , "for (let x = 6; (x < 60); let x = (x + 1)) {x}"},
			{"for (let x = 6; (x != 60); t(x)) {x}"            , "for (let x = 6; (x != 60); t(x)) {x}"},

		};

		for (auto& test : tests)
		{
			Lexer lexer(test.input);
			Parser parser(lexer);

			auto program = parser.ParseProgram();
			auto errors = parser.Errors();
			REQUIRE(errors.size() == 0);

			REQUIRE(program->Statements.size() == 1);
			auto forStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
			REQUIRE(forStatement != nullptr);
			REQUIRE(forStatement->TokenLiteral() == "for");

			REQUIRE(forStatement->ToString() == test.expectedValue);
		}

	}
		
}