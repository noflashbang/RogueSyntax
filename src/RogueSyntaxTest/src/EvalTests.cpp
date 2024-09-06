#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>

IObject* EvalTest(const std::string& input)
{
	Evaluator eval;
	Lexer lexer(input);
	Parser parser(lexer);
	
	auto program = parser.ParseProgram();
	auto errors = parser.Errors();
	REQUIRE(errors.size() == 0);

	IObject* result = eval.Eval(program);
	return result;
}

bool TestIntegerObject(IObject* obj, const int32_t expected)
{
	if (obj->Type() != ObjectType::INTEGER_OBJ)
	{
		throw new std::invalid_argument(std::format("Object is not an integer -> Expected {} GOT {}", ObjectType::INTEGER_OBJ.Name, obj->Type().Name));
		return false;
	}

	auto converted = dynamic_cast<IntegerObj*>(obj);
	auto actualValue = converted->Value;

	if (actualValue != expected)
	{
		throw new std::invalid_argument(std::format("Value is incorrect -> Expected {} GOT {}", expected, actualValue));
		return false;
	}
	return true;
}

bool TestEvalInteger(const std::string& input, const int32_t expected)
{
	auto result = EvalTest(input);
	return TestIntegerObject(result, expected);
}

bool TestBooleanObject(IObject* obj, const bool expected)
{
	if (obj->Type() != ObjectType::BOOLEAN_OBJ)
	{
		throw new std::invalid_argument(std::format("Object is not a boolean -> Expected {} GOT {}", ObjectType::BOOLEAN_OBJ.Name, obj->Type().Name));
		return false;
	}

	auto converted = dynamic_cast<BooleanObj*>(obj);
	auto actualValue = converted->Value;

	if (actualValue != expected)
	{
		throw new std::invalid_argument(std::format("Value is incorrect -> Expected {} GOT {}", expected, actualValue));
		return false;
	}
	return true;
}

bool TestEvalBoolean(const std::string& input, const bool expected)
{
	auto result = EvalTest(input);
	return TestBooleanObject(result, expected);
}

TEST_CASE("Eval Tests")
{
	SECTION("Simple Integer Type")
	{
		auto tests = std::vector<std::pair<std::string, int32_t>>
		{
			{"5", 5},
			{"10", 10},
			//{"-5", -5},
			//{"-10", -10},
			//{"5 + 5 + 5 + 5 - 10", 10},
			//{"2 * 2 * 2 * 2 * 2", 32},
			//{"-50 + 100 + -50", 0},
			//{"5 * 2 + 10", 20},
			//{"5 + 2 * 10", 25},
			//{"20 + 2 * -10", 0},
			//{"50 / 2 * 2 + 10", 60},
			//{"2 * (5 + 10)", 30},
			//{"3 * 3 * 3 + 10", 37},
			//{"3 * (3 * 3) + 10", 37},
			//{"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50}
		};

		for (auto& test : tests)
		{
			REQUIRE(TestEvalInteger(test.first, test.second));
		}
	}

	SECTION("Simple Boolean Type")
	{
		auto tests = std::vector<std::pair<std::string, bool>>
		{
			{"true", true},
			{"false", false},
			//{"1 < 2", true},
			//{"1 > 2", false},
			//{"1 < 1", false},
			//{"1 > 1", false},
			//{"1 == 1", true},
			//{"1 != 1", false},
			//{"1 == 2", false},
			//{"1 != 2", true},
			//{"true == true", true},
			//{"false == false", true},
			//{"true == false", false},
			//{"true != false", true},
			//{"false != true", true},
			//{"(1 < 2) == true", true},
			//{"(1 < 2) == false", false},
			//{"(1 > 2) == true", false},
			//{"(1 > 2) == false", true}
		};

		for (auto& test : tests)
		{
			REQUIRE(TestEvalBoolean(test.first, test.second));
		}
	}
}