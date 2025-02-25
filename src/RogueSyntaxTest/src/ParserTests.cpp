
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

bool TestDecimalLiteral(const IExpression* expression, float expected, const std::string& expectedLiteral)
{
	auto decimal = dynamic_cast<const DecimalLiteral*>(expression);
	if (decimal == nullptr)
	{
		throw std::invalid_argument(std::format("Bad expression -> Expected DecimalLiteral GOT {}", expression->ToString()));
		return false;
	}

	if (abs(decimal->Value - expected) > FLT_EPSILON)
	{
		throw std::invalid_argument(std::format("Bad Value -> Expected {} GOT {}", expected, decimal->Value));
		return false;
	}

	if (decimal->TokenLiteral() != expectedLiteral)
	{
		throw std::invalid_argument(std::format("Bad Literal -> Expected {} GOT {}", expected, decimal->Value));
		return false;
	}

	return true;
}

bool TestStringLiteral(const IExpression* expression, const std::string& expected, const std::string& expectedLiteral)
{
	auto str = dynamic_cast<const StringLiteral*>(expression);
	if (str == nullptr)
	{
		throw std::invalid_argument(std::format("Bad expression -> Expected StringLiteral GOT {}", expression->ToString()));
		return false;
	}

	if (str->Value != expected)
	{
		throw std::invalid_argument(std::format("Bad Value -> Expected {} GOT {}", expected, str->Value));
		return false;
	}

	if (str->TokenLiteral() != expectedLiteral)
	{
		throw std::invalid_argument(std::format("Bad Literal -> Expected {} GOT {}", expected, str->Value));
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

bool TestNonLiteralExpression(const IExpression* expression, const std::string& expected)
{
	return expression->ToString() == expected;
}

bool TestArrayLiteral(const IExpression* expression, std::vector<std::string> elements)
{
	auto arr = dynamic_cast<const ArrayLiteral*>(expression);
	if (arr == nullptr)
	{
		throw std::invalid_argument(std::format("Bad expression -> Expected ArrayLiteral GOT {}", expression->ToString()));
		return false;
	}

	auto size = elements.size();
	if (arr->Elements.size() != size)
	{
		throw std::invalid_argument(std::format("Bad Value -> Expected {} GOT {}", size, arr->Elements.size()));
		return false;
	}

	for (size_t i = 0; i < size; i++)
	{
		if (!TestNonLiteralExpression(arr->Elements[i], elements[i]))
		{
			return false;
		}
	}

	return true;
}

bool TestHashLiteral(const IExpression* expression, std::vector<std::string> elements)
{
	auto hash = dynamic_cast<const HashLiteral*>(expression);
	if (hash == nullptr)
	{
		throw std::invalid_argument(std::format("Bad expression -> Expected HashLiteral GOT {}", expression->ToString()));
		return false;
	}

	auto size = elements.size();
	if (hash->Elements.size() != size)
	{
		throw std::invalid_argument(std::format("Bad Value -> Expected {} GOT {}", size, hash->Elements.size()));
		return false;
	}
	int i = 0;
	for (auto [first, second] : hash->Elements)
	{
		auto pair = std::format("({}):({})", first->ToString(), second->ToString());
		REQUIRE(pair == elements[i]);
		i++;
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

	if (!TestLiteralExpression(infix->Left, left))
	{
		return false;
	}

	if (infix->Operator != op)
	{
		throw std::invalid_argument(std::format("Bad Operator -> Expected {} GOT {}", op, infix->Operator));
		return false;
	}

	if (!TestLiteralExpression(infix->Right, right))
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

	if (!TestLiteralExpression(prefix->Right, right))
	{
		return false;
	}

	return true;
}

TEST_CASE("Test Identifier")
{
	std::string input = "foobar;";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0]);
	REQUIRE(expressionStatement != nullptr);

	REQUIRE(TestIdentifier(expressionStatement->Expression, "foobar"));
}

TEST_CASE("Test Integer Literal")
{
	std::string input = "5;";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0]);
	REQUIRE(expressionStatement != nullptr);
	auto integer = dynamic_cast<const IntegerLiteral*>(expressionStatement->Expression);

	REQUIRE(TestIntegerLiteral(expressionStatement->Expression, 5));
}

TEST_CASE("Test Decimal Literal")
{
	std::string input = "5.5;";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	if (errors.size() != 0)
	{
		for (auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0]);
	REQUIRE(expressionStatement != nullptr);
	auto integer = dynamic_cast<const IntegerLiteral*>(expressionStatement->Expression);

	REQUIRE(TestDecimalLiteral(expressionStatement->Expression, 5.5f, "5.5"));
}

TEST_CASE("Test String Literal")
{
	std::string input = "\"Hello World\";";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	if (errors.size() != 0)
	{
		for (auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0]);
	REQUIRE(expressionStatement != nullptr);

	REQUIRE(TestStringLiteral(expressionStatement->Expression, "Hello World", "\"Hello World\""));
}

TEST_CASE("Test Boolean Literal")
{
	std::string input = "true;";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0]);
	REQUIRE(expressionStatement != nullptr);

	REQUIRE(TestBooleanLiteral(expressionStatement->Expression, true));
}

TEST_CASE("Test Array Literal")
{
	std::string input = "[4, 4 + 4, 4 * 4];";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0]);
	REQUIRE(expressionStatement != nullptr);

	REQUIRE(TestArrayLiteral(expressionStatement->Expression, {"4","(4 + 4)","(4 * 4)"}));
}

TEST_CASE("Test Hash Literal")
{
	std::string input = "{1:4,2:5}; ";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();

	if (errors.size() != 0)
	{
		for (auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0]);
	REQUIRE(expressionStatement != nullptr);

	REQUIRE(TestHashLiteral(expressionStatement->Expression, { "(1):(4)","(2):(5)"}));
}


TEST_CASE("Test Prefix Expression BANG")
{
	std::string input = "!5;";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0]);
	REQUIRE(expressionStatement != nullptr);

	auto prefix = dynamic_cast<const PrefixExpression*>(expressionStatement->Expression);
}

TEST_CASE("Test Prefix Expression MINUS")
{
	std::string input = "-15;";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0]);
	REQUIRE(expressionStatement != nullptr);

	REQUIRE(TestPrefixExpression(expressionStatement->Expression, "-", "15"));
}

TEST_CASE("Test postfix statements")
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

		auto program = parser.ParseProgram("TESTPRG");
		auto errors = parser.Errors();
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto assignStatement = dynamic_cast<LetStatement*>(program->Statements[0]);
		REQUIRE(assignStatement != nullptr);
		REQUIRE(assignStatement->ToString() == test.expected);
	}
}

TEST_CASE("Test Infix Expression")
{
	std::string input = "5 + 5;";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0]);
	REQUIRE(expressionStatement != nullptr);

	REQUIRE(TestInfixExpression(expressionStatement->Expression, "5", "+", "5"));
}

TEST_CASE("Test Operator Precedence Parsing")
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
		{"add(a + b + c * d / f + g)", "add((((a + b) + ((c * d) / f)) + g))"},
		{"a * [1, 2, 3, 4][b * c] * d", "((a * ([1, 2, 3, 4][(b * c)])) * d)"},
		{"add(a * b[2], b[1], 2 * [1, 2][1])", "add((a * (b[2])), (b[1]), (2 * ([1, 2][1])))"},
	};

	for (auto& test : tests)
	{
		Lexer lexer(test.input);
		Parser parser(lexer);
		
		INFO(test.input);

		auto program = parser.ParseProgram("TESTPRG");
		auto errors = parser.Errors();
		if (errors.size() != 0)
		{
			for (auto& error : errors)
			{
				UNSCOPED_INFO(error);
			}
		}
		REQUIRE(errors.size() == 0);
		REQUIRE(program->ToString() == test.expected);
	}
}

TEST_CASE("Test Parsing Infix Expressions")
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
		{"5 & 5;", "5", "&", "5"},
		{"5 < 5;", "5", "<", "5"},
		{"5 == 5;", "5", "==", "5"},
		{"5 != 5;", "5", "!=", "5"},
		{"true == true", "true", "==", "true"},
		{"true != false", "true", "!=", "false"},
		{"false == false", "false", "==", "false"},
		{"5 & 5;", "5", "&", "5"},
		{"5 | 5;", "5", "|", "5"},
		{"5 ^ 5;", "5", "^", "5"},
		{"5 << 5;", "5", "<<", "5"},
		{"5 >> 5;", "5", ">>", "5"},
		{"true && false;", "true", "&&", "false"},
		{"true || false;", "true", "||", "false"},
	};

	for (auto& test : tests)
	{
		Lexer lexer(test.input);
		Parser parser(lexer);

		auto program = parser.ParseProgram("TESTPRG");
		auto errors = parser.Errors();

		INFO(test.input);

		if (errors.size() != 0)
		{
			for (auto& error : errors)
			{
				UNSCOPED_INFO(error);
			}
		}
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0]);
		REQUIRE(expressionStatement != nullptr);

		REQUIRE(TestInfixExpression(expressionStatement->Expression, test.left, test.op, test.right));
	}
}

TEST_CASE("Test Let Statement")
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

		auto program = parser.ParseProgram("TESTPRG");
		auto errors = parser.Errors();

		INFO(test.input);

		if (errors.size() != 0)
		{
			for (auto& error : errors)
			{
				UNSCOPED_INFO(error);
			}
		}
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto letStatement = dynamic_cast<LetStatement*>(program->Statements[0]);
		REQUIRE(letStatement != nullptr);

		REQUIRE(letStatement->TokenLiteral() == "let");
		REQUIRE(TestIdentifier(letStatement->Name, test.expectedIdentifier));
		REQUIRE(TestLiteralExpression(letStatement->Value, test.expectedValue));
	}
}

TEST_CASE("Test Errors")
{
	std::string input = "let x 5;";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	if (errors.size() != 0)
	{
		for (auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() != 0);
}

TEST_CASE("Test Return Statement")
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

		auto program = parser.ParseProgram("TESTPRG");
		auto errors = parser.Errors();
		if (errors.size() != 0)
		{
			for (auto& error : errors)
			{
				UNSCOPED_INFO(error);
			}
		}
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto returnStatement = dynamic_cast<ReturnStatement*>(program->Statements[0]);
		REQUIRE(returnStatement != nullptr);

		REQUIRE(returnStatement->TokenLiteral() == "return");
		REQUIRE(TestLiteralExpression(returnStatement->ReturnValue, test.expectedValue));
	}
}

TEST_CASE("Test If Expression")
{
	std::string input = "if (x < y) { x }";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	if (errors.size() != 0)
	{
		for (auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);

	auto ifStatement = dynamic_cast<IfStatement*>(program->Statements[0]);
	REQUIRE(ifStatement != nullptr);
	REQUIRE(ifStatement->TokenLiteral() == "if");
		
	REQUIRE(TestInfixExpression(ifStatement->Condition, "x", "<", "y"));

	auto blockStatement = dynamic_cast<const BlockStatement*>(ifStatement->Consequence);
	REQUIRE(blockStatement != nullptr);

	auto statement = dynamic_cast<ExpressionStatement*>(blockStatement->Statements[0]);
	REQUIRE(statement != nullptr);
	REQUIRE(TestIdentifier(statement->Expression, "x"));
}

TEST_CASE("Test If Else Expression")
{
	std::string input = "if (x < y) { x } else { y }";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	if (errors.size() != 0)
	{
		for (auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);

	auto ifStatement = dynamic_cast<IfStatement*>(program->Statements[0]);
	REQUIRE(ifStatement != nullptr);
	REQUIRE(ifStatement->TokenLiteral() == "if");

	REQUIRE(TestInfixExpression(ifStatement->Condition, "x", "<", "y"));

	auto consequence = dynamic_cast<const BlockStatement*>(ifStatement->Consequence);
	REQUIRE(consequence != nullptr);

	auto statement = dynamic_cast<ExpressionStatement*>(consequence->Statements[0]);
	REQUIRE(statement != nullptr);
	REQUIRE(TestIdentifier(statement->Expression, "x"));

	auto alternative = dynamic_cast<const BlockStatement*>(ifStatement->Alternative);
	REQUIRE(alternative != nullptr);

	statement = dynamic_cast<ExpressionStatement*>(alternative->Statements[0]);
	REQUIRE(statement != nullptr);
	REQUIRE(TestIdentifier(statement->Expression, "y"));

}

TEST_CASE("Test Function Literal")
{
	std::string input = "fn(x, y) { x + y; }";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	if (errors.size() != 0)
	{
		for (auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0]);
	REQUIRE(expressionStatement != nullptr);

	auto function = dynamic_cast<const FunctionLiteral*>(expressionStatement->Expression);
	REQUIRE(function != nullptr);
	REQUIRE(function->TokenLiteral() == "fn");

	REQUIRE(function->Parameters.size() == 2);

	auto identifier = dynamic_cast<Identifier*>(function->Parameters[0]);
	REQUIRE(identifier != nullptr);
	REQUIRE(identifier->Value == "x");

	identifier = dynamic_cast<Identifier*>(function->Parameters[1]);
	REQUIRE(identifier != nullptr);
	REQUIRE(identifier->Value == "y");
		
	auto blockStatement = dynamic_cast<const BlockStatement*>(function->Body);
	REQUIRE(blockStatement != nullptr);

	REQUIRE(blockStatement->Statements.size() == 1);
	auto statement = dynamic_cast<ExpressionStatement*>(blockStatement->Statements[0]);
	REQUIRE(statement != nullptr);

	REQUIRE(TestInfixExpression(statement->Expression, "x", "+", "y"));
}

TEST_CASE("Test Call Expression")
{
	std::string input = "add(1, 2 * 3, 4 + 5);";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	if (errors.size() != 0)
	{
		for (auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0]);
	REQUIRE(expressionStatement != nullptr);

	auto call = dynamic_cast<const CallExpression*>(expressionStatement->Expression);
	REQUIRE(call != nullptr);
	REQUIRE(call->Function->TokenLiteral() == "add");

	REQUIRE(call->Arguments.size() == 3);

	REQUIRE(TestLiteralExpression(call->Arguments[0], "1"));
	REQUIRE(TestInfixExpression(call->Arguments[1], "2", "*", "3"));
	REQUIRE(TestInfixExpression(call->Arguments[2], "4", "+", "5"));
}

TEST_CASE("Test Index expression")
{
	std::string input = "array[1 + 1];";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	if (errors.size() != 0)
	{
		for (auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto expressionStatement = dynamic_cast<ExpressionStatement*>(program->Statements[0]);
	REQUIRE(expressionStatement != nullptr);

	auto index = dynamic_cast<const IndexExpression*>(expressionStatement->Expression);
	REQUIRE(index != nullptr);
	REQUIRE(index->Left->TokenLiteral() == "array");
	REQUIRE(TestInfixExpression(index->Index, "1", "+", "1"));
}

TEST_CASE("Test While expression")
{
	std::string input = "while (x < y) { let y = y + 1; }";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	if (errors.size() != 0)
	{
		for (auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);

	auto whileStatement = dynamic_cast<WhileStatement*>(program->Statements[0]);
	REQUIRE(whileStatement != nullptr);
	REQUIRE(whileStatement->TokenLiteral() == "while");

	REQUIRE(TestInfixExpression(whileStatement->Condition, "x", "<", "y"));

	auto blockStatement = dynamic_cast<const BlockStatement*>(whileStatement->Action);
	REQUIRE(blockStatement != nullptr);

	auto statement = dynamic_cast<LetStatement*>(blockStatement->Statements[0]);
	REQUIRE(statement != nullptr);
	REQUIRE(statement->Value->ToString() == "(y + 1)");
}

TEST_CASE("Test Break Statement")
{
	std::string input = "break;";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	if (errors.size() != 0)
	{
		for (auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto breakStatement = dynamic_cast<BreakStatement*>(program->Statements[0]);
	REQUIRE(breakStatement != nullptr);
	REQUIRE(breakStatement->TokenLiteral() == "break");
}

TEST_CASE("Test Continue Statement")
{
	std::string input = "continue;";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	if (errors.size() != 0)
	{
		for (auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto continueStatement = dynamic_cast<ContinueStatement*>(program->Statements[0]);
	REQUIRE(continueStatement != nullptr);
	REQUIRE(continueStatement->TokenLiteral() == "continue");
}

TEST_CASE("Test assignemnt")
{
	std::string input = "x = 5;";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	if (errors.size() != 0)
	{
		for (auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto assignStatement = dynamic_cast<LetStatement*>(program->Statements[0]);
	REQUIRE(assignStatement != nullptr);
	REQUIRE(assignStatement->Name->ToString() == "x");
	REQUIRE(assignStatement->Value->ToString() == "5");
}

TEST_CASE("Test function assignemnt")
{
	std::string input = "x = fn(x) { return x * x;};";
	Lexer lexer(input);
	Parser parser(lexer);

	auto program = parser.ParseProgram("TESTPRG");
	auto errors = parser.Errors();
	if (errors.size() != 0)
	{
		for (auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	REQUIRE(program->Statements.size() == 1);
	auto assignStatement = dynamic_cast<LetStatement*>(program->Statements[0]);
	REQUIRE(assignStatement != nullptr);
	REQUIRE(assignStatement->Name->ToString() == "x");
	REQUIRE(assignStatement->Value->ToString() == "fn(x){return (x * x);}");
}

TEST_CASE("Test for parsing")
{
	struct Test
	{
		std::string input;
		std::string expectedValue;
	};

	std::vector<Test> tests = {
		{"for (let x = 6; (x < 60); let x = (x + 1)) {x}" , "for (let x = 6; (x < 60); let x = (x + 1)) {x}"},
		{"for(x=6; x<60;x++){ x; }"                       , "for (let x = 6; (x < 60); let x = (x + 1)) {x}"},
		{"for(let x=6; x<60;x = x+1){ x; }"               , "for (let x = 6; (x < 60); let x = (x + 1)) {x}"},
		{"for(x=6; x!=60; x = t(x)){ x; }"                     , "for (let x = 6; (x != 60); let x = t(x)) {x}"},
		{"for (let x = 6; (x < 60); let x = (x + 1)) {x}" , "for (let x = 6; (x < 60); let x = (x + 1)) {x}"},
		{"for (let x = 6; (x != 60); x = t(x)) {x}"           , "for (let x = 6; (x != 60); let x = t(x)) {x}"},

	};

	for (auto& test : tests)
	{
		Lexer lexer(test.input);
		Parser parser(lexer);

		UNSCOPED_INFO(test.input);
		auto program = parser.ParseProgram("TESTPRG");
		auto errors = parser.Errors();
		if (errors.size() != 0)
		{
			for (auto& error : errors)
			{
				UNSCOPED_INFO(error);
			}
		}
		REQUIRE(errors.size() == 0);

		REQUIRE(program->Statements.size() == 1);
		auto forStatement = dynamic_cast<ForStatement*>(program->Statements[0]);
		REQUIRE(forStatement != nullptr);
		REQUIRE(forStatement->TokenLiteral() == "for");

		REQUIRE(forStatement->ToString() == test.expectedValue);
	}
}
	
TEST_CASE("TEST ASSIGN OPERATORS")
{
	struct Test
	{
		std::string input;
		std::string expectedValue;
	};

	std::vector<Test> tests = {
		{"x += 5;", "let x = (x + 5);"},
		{"x -= 5;", "let x = (x - 5);"},
		{"x *= 5;", "let x = (x * 5);"},
		{"x /= 5;", "let x = (x / 5);"},
		{"x %= 5;", "let x = (x % 5);"},
	};

	for (auto& test : tests)
	{
		Lexer lexer(test.input);
		Parser parser(lexer);

		auto program = parser.ParseProgram("TESTPRG");
		auto errors = parser.Errors();
		if (errors.size() != 0)
		{
			for (auto& error : errors)
			{
				UNSCOPED_INFO(error);
			}
		}
		REQUIRE(errors.size() == 0);

		CAPTURE(test.input);
		REQUIRE(program->Statements.size() == 1);
		auto stmt = dynamic_cast<LetStatement*>(program->Statements[0]);

		REQUIRE(stmt != nullptr);
		REQUIRE(stmt->ToString() == test.expectedValue);
	}

}