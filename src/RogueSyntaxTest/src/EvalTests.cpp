#include <RogueSyntaxCore.h>
#include <RogueSyntax.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

bool TestIntegerObject(const IObject* obj, const int32_t expected)
{
	if (!obj->IsThisA<IntegerObj>())
	{
		if (obj->IsThisA<ErrorObj>())
		{
			auto errorObj = dynamic_cast<const ErrorObj*>(obj);
			UNSCOPED_INFO(errorObj->Message);
		}
		throw std::invalid_argument(std::format("Object is not an integer -> Expected {} GOT {}", "IntegerObj", obj->TypeName()));
		return false;
	}

	auto converted = dynamic_cast<const IntegerObj*>(obj);
	auto actualValue = converted->Value;

	if (actualValue != expected)
	{
		throw std::invalid_argument(std::format("Value is incorrect -> Expected {} GOT {}", expected, actualValue));
		return false;
	}
	return true;
}

bool TestEvalInteger(EvaluatorType type, const std::string& input, const int32_t expected)
{
	RogueSyntax syn;
	auto result = syn.QuickEval(type, input);

	if (result->IsThisA<NullObj>() && expected == 0)
	{
		return true;
	}
	return TestIntegerObject(result, expected);
}

bool TestBooleanObject(const IObject* obj, const bool expected)
{
	if (!obj->IsThisA<BooleanObj>())
	{
		throw std::invalid_argument(std::format("Object is not a boolean -> Expected {} GOT {}", "BooleanObj", obj->TypeName()));
		return false;
	}

	auto converted = dynamic_cast<const BooleanObj*>(obj);
	auto actualValue = converted->Value;

	if (actualValue != expected)
	{
		throw std::invalid_argument(std::format("Value is incorrect -> Expected {} GOT {}", expected, actualValue));
		return false;
	}
	return true;
}

bool TestEvalBoolean(EvaluatorType type, const std::string& input, const bool expected)
{
	RogueSyntax syn;
	auto result = syn.QuickEval(type, input);
	return TestBooleanObject(result, expected);
}

bool TestEvalInspect(EvaluatorType type, const std::string& input, const std::string& expected)
{
	RogueSyntax syn;
	auto result = syn.QuickEval(type, input);
	return result->Inspect() == expected;
}
	
TEST_CASE("Simple Integer Type")
{
	auto [eng] = GENERATE(table<EvaluatorType>( { EvaluatorType::Stack, EvaluatorType::Recursive }));
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
		{ "(5 + 10 * 2 + 15 / 3) * 2 + -10", 50 },
		{"7 % 3",1},
		{"2 | 3",3},
		{"2 & 3",2},
		{"2 ^ 3",1},
		{"2 << 3",16},
		{"8 >> 3",1},
		{"~2",-3}
	}));

	
	CAPTURE(input);
	REQUIRE(TestEvalInteger(eng, input, expected));
}

TEST_CASE("Simple Boolean Type")
{
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
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
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
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
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
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
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
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
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
	auto [input, expected] = GENERATE(table<std::string, std::string>(
	{
		{"-true", "unknown operator: -class BooleanObj"},
		{"true + false;", "unknown operator: class BooleanObj + class BooleanObj"},
		{"5; true + false; 5", "unknown operator: class BooleanObj + class BooleanObj"},
		{"if (10 > 1) { true + false; }", "unknown operator: class BooleanObj + class BooleanObj"},
		{
			"if (10 > 1) {"
			"if (10 > 1) {"
			"return true + false;"
			"}"
			"return 1;"
			"}",
			"unknown operator: class BooleanObj + class BooleanObj"
		},
		//{"foobar", "identifier not found: foobar"}
	}));

	CAPTURE(input);
	REQUIRE(TestEvalInspect(eng, input, expected));
}

TEST_CASE("Test Let Statements")
{
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
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


TEST_CASE("Test Function Eval")
{
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
	auto [input, expected] = GENERATE(table<std::string, int32_t>(
	{
		{"let identity = fn(x) { return x; }; identity(5);", 5},
		{"let identity = fn(x) { return x; }; identity(5);", 5},
		{"let double = fn(x) { return x * 2; }; double(5);", 10},
		{"let add = fn(x, y) { return x + y; }; add(5, 5);", 10},
		{"let add = fn(x, y) { return x + y; }; add(5 + 5, add(5, 5));", 20},
		{"let x = fn(a) {return a*a;}; let y = fn(b) {return b*b;}; x(y(5));", 625},
		{"let x = fn(a) {return a*a;}; let y = fn(b) {return b*b;}; x(y(5+x(2)));", 6561},
		{"fn(x) { x; }(5)", 5}
		}));

	CAPTURE(input);
	REQUIRE(TestEvalInteger(eng, input, expected));
}

TEST_CASE("Test While Loop")
{
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
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
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
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
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
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

TEST_CASE("Decimal and String tests")
{
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
	auto [input, expected] = GENERATE(table<std::string, std::string>(
		{
			{"let a = 5.5; a;", "5.500000"},
			{"let a = 5d; let b = a; a = 10d; b;", "5.000000"},
			{"let a = 5.5; let b = a; a = 10.5; a;", "10.500000"},
			{"let a = 5.5; let b = 10.5; a + b;", "16.000000"},
			{"let a = 5.5; let b = 10.5; a - b;", "-5.000000"},
			{"let a = 2.5d; let b = 4; a * b;", "10.000000"},
			{"let a = 10d; let b = 2.5d; a / b;", "4.000000"},
			{"let a = 5.5; let b = 10.5; a == b;", "false"},
			{"let a = 5.5; let b = 10.5; a != b;", "true"},
			{"let a = 5.5; let b = 10.5; a < b;", "true"},
			{"let a = 5.5; let b = 10.5; a > b;", "false"},
			{"let a = 5.5; let b = 10.5; a == 5.5;", "true"},
			{"let a = 5.5; let b = 10.5; a != 5.5;", "false"},
			{"let a = 5.5; let b = 10.5; a == 10.5;", "false"},
			{"let a = 5.5; let b = 10.5; a != 10.5;", "true"},
			{"a = 6.6; a;", "6.600000"},
			{"let a = \"Hello\"; a;", "Hello"},
			{"let a = \"Hello\"; let b = a; a = \"World\"; b;", "Hello"},
			{"let a = \"Hello\"; let b = a; a = \"World\"; a;", "World"},
			{"let a = \"Hello\"; let b = \"World\"; a + b;", "HelloWorld"},
			{"a = \"Hello\"; a;", "Hello"}
		}));

	
	CAPTURE(input);
	REQUIRE(TestEvalInspect(eng, input, expected));
}

TEST_CASE("Array Literal Tests")
{
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
	auto [input, expected] = GENERATE(table<std::string, std::string>(
		{
			{"[]", "[]"},
			{"[1, 2, 3]", "[1, 2, 3]"},
			{"[1 + 2, 3 * 4, 5 + 6]", "[3, 12, 11]"}
		}));

	
	CAPTURE(input);
	REQUIRE(TestEvalInspect(eng, input, expected));
}

TEST_CASE("Hash Literal Tests")
{
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
	auto [input, expected] = GENERATE(table<std::string, std::string>(
		{
			{"{}", "{}"},
			{"{1: 2, 2: 3}", "{1: 2, 2: 3}"},
			{"{1 + 1: 2 * 2, 3 + 3: 4 * 4}", "{2: 4, 6: 16}"}
		}));

	
	CAPTURE(input);
	REQUIRE(TestEvalInspect(eng, input, expected));
}

TEST_CASE("Index Array Expression Test")
{
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
	auto [input, expected] = GENERATE(table<std::string, std::string>(
		{
			{"[1, 2, 3][0]", "1"},
			{"[1, 2, 3][1]", "2"},
			{"[1, 2, 3][2]", "3"},
			{"let i = 0; [1][i];", "1"},
			{"[1, 2, 3][1 + 1];", "3"},
			{"let myArray = [1, 2, 3]; myArray[2];", "3"},
			{"let myArray = [1, 2, 3]; myArray[0] + myArray[1] + myArray[2];", "6"},
			{"let myArray = [1, 2, 3]; let i = myArray[0]; myArray[i];", "2"},
			{"[1, 2, 3][3]", "null"},
			{"[1, 2, 3][-1]", "null"}
		}));

	
	CAPTURE(input);
	REQUIRE(TestEvalInspect(eng, input, expected));
}

TEST_CASE("Index Hash Expression Test")
{
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
	auto [input, expected] = GENERATE(table<std::string, std::string>(
		{
			{"{1: 1, 2: 2}[1]", "1"},
			{"{1: 1, 2: 2}[2]", "2"},
			{"let i = 1; {1: 1, 2: 2}[i];", "1"},
			{"{1: 1, 2: 2}[1 + 1];", "2"},
			{"let myHash = {1: 1, 2: 2}; myHash[2];", "2"},
			{"let myHash = {1: 1, 2: 2}; myHash[1 + 1];", "2"},
			{"{1: 1, 2: 2}[3]", "null"},
			{"{1: 1, 2: 2}[-1]", "null"}
		}));

	
	CAPTURE(input);
	REQUIRE(TestEvalInspect(eng, input, expected));
}

TEST_CASE("Index Assignment Test")
{
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
	auto [input, expected] = GENERATE(table<std::string, std::string>(
		{
			{"let x = {1: 1, 2: 2}; x[1] = 2; x[1]", "2"},
			{"let x = {1: 1, 2: 2}; x[2] = 3; x[2]", "3"},
			{"let i = [1,2,3,4,5]; i[1] = 9; i;", "[1, 9, 3, 4, 5]"},
			{"let h = {1: 1, 2: 2}; h[3] = 5; h[3]", "5"},
			{"let x = {1: 1, 2: 2}; let x[1] = 2; x[1]", "2"},
			{"let x = {1: 1, 2: 2}; let x[2] = 3; x[2]", "3"},
			{"let i = [1,2,3,4,5];  let i[1] = 9; i;", "[1, 9, 3, 4, 5]"},
			{"let h = {1: 1, 2: 2}; let h[3] = 5; h[3]", "5"},
		}));

	
	CAPTURE(input);
	REQUIRE(TestEvalInspect(eng, input, expected));
}

TEST_CASE("Coerce Tests")
{
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
	auto [input, expected] = GENERATE(table<std::string, std::string>(
		{
			{"5 + 5.5;", "10.500000"},
			{"5.5 + 5;", "10.500000"},
			{"5 == 5.5;", "false"},
			{"5.5 == 5;", "false"},
			{"5 != 5.5;", "true"},
			{"5.5 != 5;", "true"},
			{"5 < 5.5;", "true"},
			{"5.5 < 5;", "false"},
			{"5 > 5.5;", "false"},
			{"5.5 > 5;", "true"},
			{"5 == true;", "true"},
			{"5 != true;", "false"}
		}));

	
	CAPTURE(input);
	REQUIRE(TestEvalInspect(eng, input, expected));
}

TEST_CASE("Built In method test")
{
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
	auto [input, expected] = GENERATE(table<std::string, std::string>(
		{
			{"len(\"\");", "0"},
			{"len(\"four\");", "4"},
			{"len(\"hello world\");", "11"},
			{"len(1);", "ERROR: argument to `len` not supported, got class IntegerObj"},
			{"len(\"one\", \"two\");", "ERROR: wrong number of arguments. got=2, wanted=1"},
			{"len([1, 2, 3]);", "3"},
			{"len([]);", "0"},
			{"first([1, 2, 3]);", "1"},
			{"first([]);", "null"},
			{"first(1);", "ERROR: argument to `first` must be ARRAY, got class IntegerObj"},
			{"last([1, 2, 3]);", "3"},
			{"last([]);", "null"},
			{"last(1);", "ERROR: argument to `last` must be ARRAY, got class IntegerObj"},
			{"rest([1, 2, 3]);", "[2, 3]"},
			{"rest([]);", "null"},
			{"push([], 1);", "[1]"},
			{"push(1, 1);", "ERROR: argument to `push` must be ARRAY, got class IntegerObj"},
			{"let acc = 0; let list = [1,2,3,4,5,6,7,8,9]; for(iter = 0; iter < len(list); iter++) { acc = acc + list[iter]; } acc;", "45"},
		}));

	
	CAPTURE(input);
	REQUIRE(TestEvalInspect(eng, input, expected));
}

TEST_CASE("Null Tests")
{
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
	auto [input, expected] = GENERATE(table<std::string, std::string>(
		{
			{"let a = null; a;", "null"},
			{"let a = null; let b = a; b;", "null"},
			{"let a = null; let b = a; a = 10; b;", "null"},
			{"a = null; a;", "null"},
			{"null == null;", "true"},
			{"null != null;", "false"},
			{"null == 5;", "false"},
			{"null != 5;", "true"},
			{"null == true;", "false"},
			{"null != true;", "true"},
			{"null == false;", "false"},
			{"null != false;", "true"},
			{"null == \"Hello\";", "false"},
			{"null != \"Hello\";", "true"},
			{"null == [1, 2, 3];", "false"},
			{"null != [1, 2, 3];", "true"},
			{"null == {\"key\": \"value\"};", "false"},
			{"null != {\"key\": \"value\"};", "true"},
			{"null == fn(x) { x; };", "false"},
			{"null != fn(x) { x; };", "true"}
		}));

	
	CAPTURE(input);
	REQUIRE(TestEvalInspect(eng, input, expected));
}

TEST_CASE("Incrementer Tests")
{
	auto [eng] = GENERATE(table<EvaluatorType>({ EvaluatorType::Stack, EvaluatorType::Recursive }));
	auto [input, expected] = GENERATE(table<std::string, std::string>(
		{
			{"let a = 1; a++; a;", "2"},
			{"let a = 1; a--; a;", "0"},
			{"let a = [1,2,3]; a[1]++; a[1];", "3"},
			{"let a = {1:2,3:4}; a[1]++; a[1];", "3"},
			{"let a = {1:2,3:4}; a[1]++; a;", "{1: 3, 3: 4}"},
		}));

	CAPTURE(input);
	REQUIRE(TestEvalInspect(eng, input, expected));
}

#ifdef DO_BENCHMARK

TEST_CASE("BENCHMARK STACK EVALUATOR")
{
	auto eng = Evaluator::New(EvaluatorType::Stack);
	auto input = "let x = 0; for (let i = 0; i < 100; i = i + 1) { x = x + i; }; x;";
	auto expected = 4950;
	
	SECTION("BENCHMARK STACK EVALUATOR - VERIFY")
	{
		
		REQUIRE(TestEvalInteger(eng, input, expected));
	}
	
	SECTION("BENCHMARK STACK EVALUATOR - TIME")
	{
		BENCHMARK("BENCHMARK STACK EVALUATOR")
		{
			
			return TestEvalInteger(eng, input, expected);
		};
	}
}

TEST_CASE("BENCHMARK RECURSIVE EVALUATOR")
{
	auto eng = Evaluator::New(EvaluatorType::Recursive);
	auto input = "let x = 0; for (let i = 0; i < 100; i = i + 1) { x = x + i; }; x;";
	auto expected = 4950;

	SECTION("BENCHMARK RECURSIVE EVALUATOR - VERIFY")
	{
		
		REQUIRE(TestEvalInteger(eng, input, expected));
	}

	SECTION("BENCHMARK RECURSIVE EVALUATOR - TIME")
	{
		BENCHMARK("BENCHMARK RECURSIVE EVALUATOR")
		{
			
			return TestEvalInteger(eng, input, expected);
		};
	}
}

#endif