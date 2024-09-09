#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace Catch {
	template<>
	struct StringMaker<ObjectType> {
		static std::string convert(ObjectType const& value) {
			return std::format("ObjType:{}",value.Name);
		}
	};
}

std::shared_ptr<IObject> EvalTest(const std::shared_ptr<Evaluator>& eval, const std::string& input)
{
	auto env = Environment::New();
	Lexer lexer(input);
	Parser parser(lexer);
	
	auto program = parser.ParseProgram();
	auto errors = parser.Errors();
	REQUIRE(errors.size() == 0);

	auto result = eval->Eval(program, env);
	return result;
}

bool TestIntegerObject(std::shared_ptr<IObject> obj, const int32_t expected)
{
	if (obj->Type() != ObjectType::INTEGER_OBJ)
	{
		if (obj->Type() == ObjectType::ERROR_OBJ)
		{
			auto errorObj = std::dynamic_pointer_cast<ErrorObj>(obj);
			UNSCOPED_INFO(errorObj->Message);
		}
		throw std::invalid_argument(std::format("Object is not an integer -> Expected {} GOT {}", ObjectType::INTEGER_OBJ.Name, obj->Type().Name));
		return false;
	}

	auto converted = std::dynamic_pointer_cast<IntegerObj>(obj);
	auto actualValue = converted->Value;

	if (actualValue != expected)
	{
		throw std::invalid_argument(std::format("Value is incorrect -> Expected {} GOT {}", expected, actualValue));
		return false;
	}
	return true;
}

bool TestEvalInteger(const std::shared_ptr<Evaluator>& eval, const std::string& input, const int32_t expected)
{
	auto result = EvalTest(eval, input);
	if (result->Type() == ObjectType::NULL_OBJ && expected == 0)
	{
		return true;
	}
	return TestIntegerObject(result, expected);
}

bool TestBooleanObject(std::shared_ptr<IObject> obj, const bool expected)
{
	if (obj->Type() != ObjectType::BOOLEAN_OBJ)
	{
		throw std::invalid_argument(std::format("Object is not a boolean -> Expected {} GOT {}", ObjectType::BOOLEAN_OBJ.Name, obj->Type().Name));
		return false;
	}

	auto converted = std::dynamic_pointer_cast<BooleanObj>(obj);
	auto actualValue = converted->Value;

	if (actualValue != expected)
	{
		throw std::invalid_argument(std::format("Value is incorrect -> Expected {} GOT {}", expected, actualValue));
		return false;
	}
	return true;
}

bool TestEvalBoolean(const std::shared_ptr<Evaluator>& eval, const std::string& input, const bool expected)
{
	auto result = EvalTest(eval, input);
	return TestBooleanObject(result, expected);
}

	
TEST_CASE("Simple Integer Type")
{
	auto [eng] = GENERATE(table<std::shared_ptr<Evaluator>>( { std::make_shared<StackEvaluator>(), std::make_shared<RecursiveEvaluator>() }));
	auto [input, expected] = GENERATE(table<std::string, int32_t>(
	{
		{ "5", 5 },
		{ "10", 10 },
		{ "-5", -5 },
		{ "-10", -10 },
		{ "5 + 5 + 5 + 5 - 10", 10 },
		{ "2 * 2 * 2 * 2 * 2", 32 },
		{ "-50 + 100 + -50", 0 },
		{ "5 * 2 + 10", 20 },
		{ "5 + 2 * 10", 25 },
		{ "20 + 2 * -10", 0 },
		{ "50 / 2 * 2 + 10", 60 },
		{ "2 * (5 + 10)", 30 },
		{ "3 * 3 * 3 + 10", 37 },
		{ "3 * (3 * 3) + 10", 37 },
		{ "(5 + 10 * 2 + 15 / 3) * 2 + -10", 50 } 
	}));

	CAPTURE(input);
	REQUIRE(TestEvalInteger(eng, input, expected));
}

TEST_CASE("Simple Boolean Type")
{
	auto [eng] = GENERATE(table<std::shared_ptr<Evaluator>>({ std::make_shared<StackEvaluator>(), std::make_shared<RecursiveEvaluator>() }));
	auto [input, expected] = GENERATE(table<std::string, bool>(
	{
		{"true", true},
		{"false", false},
		{"1 < 2", true},
		{"1 > 2", false},
		{"1 < 1", false},
		{"1 > 1", false},
		{"1 == 1", true},
		{"1 != 1", false},
		{"1 == 2", false},
		{"1 != 2", true},
		{"true == true", true},
		{"false == false", true},
		{"true == false", false},
		{"true != false", true},
		{"false != true", true},
		{"(1 < 2) == true", true},
		{"(1 < 2) == false", false},
		{"(1 > 2) == true", false},
		{"(1 > 2) == false", true}
	}));

	CAPTURE(input);
	REQUIRE(TestEvalBoolean(eng, input, expected));
}

TEST_CASE("Bang Operator")
{
	auto [eng] = GENERATE(table<std::shared_ptr<Evaluator>>({ std::make_shared<StackEvaluator>(), std::make_shared<RecursiveEvaluator>() }));
	auto [input, expected] = GENERATE(table<std::string, bool>(
	{
		{"!true", false},
		{"!false", true},
		{"!5", false},
		{"!!true", true},
		{"!!false", false},
		{"!!5", true}
	}));

	CAPTURE(input);
	REQUIRE(TestEvalBoolean(eng, input, expected));
}

TEST_CASE("Test If Else Expressions")
{
	auto [eng] = GENERATE(table<std::shared_ptr<Evaluator>>({ std::make_shared<StackEvaluator>(), std::make_shared<RecursiveEvaluator>() }));
	auto [input, expected] = GENERATE(table<std::string, int32_t>(
	{
		{"if (true) { 10 }", 10},
		{"if (false) { 10 }", 0},
		{"if (1) { 10 }", 10},
		{"if (0) { 10 }", 0},
		{"if (1 < 2) { 10 }", 10},
		{"if (1 > 2) { 10 }", 0},
		{"if (1 > 2) { 10 } else { 20 }", 20},
		{"if (1 < 2) { 10 } else { 20 }", 10}
	}));

	CAPTURE(input);
	REQUIRE(TestEvalInteger(eng, input, expected));
}

TEST_CASE("Test Return Expression")
{
	auto [eng] = GENERATE(table<std::shared_ptr<Evaluator>>({ std::make_shared<StackEvaluator>(), std::make_shared<RecursiveEvaluator>() }));
	auto [input, expected] = GENERATE(table<std::string, int32_t>(
	{
		{"return 10;", 10},
		{"return 10; 9;", 10},
		{"return 2 * 5; 9;", 10},
		{"9; return 2 * 5; 9;", 10},
		{
			"if (10 > 1) {"
			"if (10 > 1) {"
			"return 10;"
			"}"
			"return 1;"
			"}",
			10
		}
		}));

	CAPTURE(input);
	REQUIRE(TestEvalInteger(eng, input, expected));
}
TEST_CASE("Test Error Obj")
{
	auto [eng] = GENERATE(table<std::shared_ptr<Evaluator>>({ std::make_shared<StackEvaluator>(), std::make_shared<RecursiveEvaluator>() }));
	auto tests = std::vector<std::pair<std::string, std::string>>
	{
		{"5 + true;", "type mismatch: INTEGER + BOOLEAN"},
		{"5 + true; 5;", "type mismatch: INTEGER + BOOLEAN"},
		{"-true", "unknown operator: -BOOLEAN"},
		{"true + false;", "unknown operator: BOOLEAN + BOOLEAN"},
		{"5; true + false; 5", "unknown operator: BOOLEAN + BOOLEAN"},
		{"if (10 > 1) { true + false; }", "unknown operator: BOOLEAN + BOOLEAN"},
		{
			"if (10 > 1) {"
			"if (10 > 1) {"
			"return true + false;"
			"}"
			"return 1;"
			"}",
			"unknown operator: BOOLEAN + BOOLEAN"
		},
		{"foobar", "identifier not found: foobar"}
	};

	for (auto& test : tests)
	{
		auto result = EvalTest(eng, test.first);
		CAPTURE(test.first);
		REQUIRE(result->Type() == ObjectType::ERROR_OBJ);
		auto errorObj = std::dynamic_pointer_cast<ErrorObj>(result);
		REQUIRE(errorObj->Message == test.second);
	}
}

TEST_CASE("Test Let Statements")
{
	auto [eng] = GENERATE(table<std::shared_ptr<Evaluator>>({ std::make_shared<StackEvaluator>(), std::make_shared<RecursiveEvaluator>() }));
	auto [input, expected] = GENERATE(table<std::string, int32_t>(
	{
		{"let a = 5; a;", 5},
		{"let a = 5 * 5; a;", 25},
		{"let a = 5; let b = a; b;", 5},
		{"let a = 5; let b = a; let c = a + b + 5; c;", 15}
	}));

	CAPTURE(input);
	REQUIRE(TestEvalInteger(eng, input, expected));
}

TEST_CASE("Test Function Object")
{
	auto [eng] = GENERATE(table<std::shared_ptr<Evaluator>>({ std::make_shared<StackEvaluator>(), std::make_shared<RecursiveEvaluator>() }));
	auto input = "fn(x) { x + 2; };";

	
	auto result = EvalTest(eng, input);
	REQUIRE(result->Type() == ObjectType::FUNCTION_OBJ);
	auto func = dynamic_pointer_cast<FunctionObj>(result);
	REQUIRE(func->Parameters.size() == 1);
	REQUIRE(func->Parameters[0].get()->ToString() == "x");
	REQUIRE(func->Body->ToString() == "{(x + 2)}");
}

TEST_CASE("Test Function Eval")
{
	auto [eng] = GENERATE(table<std::shared_ptr<Evaluator>>({ std::make_shared<StackEvaluator>(), std::make_shared<RecursiveEvaluator>() }));
	auto [input, expected] = GENERATE(table<std::string, int32_t>(
	{
		{"let identity = fn(x) { x; }; identity(5);", 5},
		{"let identity = fn(x) { return x; }; identity(5);", 5},
		{"let double = fn(x) { x * 2; }; double(5);", 10},
		{"let add = fn(x, y) { x + y; }; add(5, 5);", 10},
		{"let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20},
		{"let x = fn(a) {return a*a;}; let y = fn(b) {return b*b;}; x(y(5));", 625},
		{"let x = fn(a) {return a*a;}; let y = fn(b) {return b*b;}; x(y(5+x(2)));", 6561},
		{"fn(x) { x; }(5)", 5}
		}));

	CAPTURE(input);
	REQUIRE(TestEvalInteger(eng, input, expected));
}

TEST_CASE("Test While Loop")
{
	auto [eng] = GENERATE(table<std::shared_ptr<Evaluator>>({ std::make_shared<StackEvaluator>(), std::make_shared<RecursiveEvaluator>() }));
	auto [input, expected] = GENERATE(table<std::string, int32_t>(
	{
		{"let i = 0; while (i < 10) { let i = i + 1; }; i;", 10},
		{"let i = 0; while (i < 10) { let i = i + 1; if(i == 5){ return i; } }; i;", 5},
		{"let i = 0; while (i < 10) { let i = i + 1; if (i == 5) { break; } }; i;", 5},
		{"let i = 0; while (i < 10) { let i = i + 1; if (i == 5) { continue; let i = 12; } }; i;", 10}
		}));

	CAPTURE(input);
	REQUIRE(TestEvalInteger(eng, input, expected));
}

TEST_CASE("Test assignment")
{
	auto [eng] = GENERATE(table<std::shared_ptr<Evaluator>>({ std::make_shared<StackEvaluator>(), std::make_shared<RecursiveEvaluator>() }));
	auto [input, expected] = GENERATE(table<std::string, int32_t>(
	{
		{"let a = 5; a = 10; a;", 10},
		{"let a = 5; let b = a; a = 10; b;", 5},
		{"let a = 5; let b = a; a = 10; a;", 10},
		{"a = 6; a;", 6}
		}));

	CAPTURE(input);
	REQUIRE(TestEvalInteger(eng, input, expected));
}

TEST_CASE("FOR tests")
{
	auto [eng] = GENERATE(table<std::shared_ptr<Evaluator>>({ std::make_shared<StackEvaluator>(), std::make_shared<RecursiveEvaluator>() }));
	auto [input, expected] = GENERATE(table<std::string, int32_t>(
	{
		{"let sum = 0; for (let i = 0; i < 10; i = i + 1) { sum = sum + i; }; sum;", 45},
		{"let sum = 0; for (let i = 0; i < 10; i = i + 1) { if (i == 5) { break; } sum = sum + i; }; sum;", 10},
		{"let sum = 0; for (let i = 0; i < 10; i = i + 1) { if (i == 5) { continue; } sum = sum + i; }; sum;", 40},
		{"sum = 0; for (i = 0; i < 10; i++) { sum = sum + i; }; sum;", 45},
		{"let plusone = fn(x){ return x+1;}; sum=0; for (i = 0; i < 10; i = plusone(i)) { if (i == 5) { break; } sum = sum + i; }; sum;", 10},
		{"let xisltten = fn(x){return x<10;}; let sum = 0; for (let i = 0; xisltten(i); i = i + 1) { if (i == 5) { continue; } sum = sum + i; }; sum;", 40}
	}));

	CAPTURE(input);
	REQUIRE(TestEvalInteger(eng, input, expected));
}