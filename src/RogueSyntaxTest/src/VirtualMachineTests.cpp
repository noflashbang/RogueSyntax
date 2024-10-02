#include <vector>
#include "CompilerTestHelpers.h"
#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

TEST_CASE("Integer Arthmetic Instructions")
{
	auto [input, expected] = GENERATE(table<std::string, ConstantValue>(
	{
		{ "5", 5 },
		{ "10", 10 },
		{"1 + 2", 3},
		{ "5 + 5 + 5 + 5 - 10", 10 },
		{ "2 * 2 * 2 * 2 * 2", 32 },
		{ "5 * 2 + 10", 20 },
		{ "5 + 2 * 10", 25 },
		{ "50 / 2 * 2 + 10", 60 },
		{ "2 * (5 + 10)", 30 },
		{ "3 * 3 * 3 + 10", 37 },
		{ "3 * (3 * 3) + 10", 37 },
		{"7 % 3",1},
		{"2 | 3",3},
		{"2 & 3",2},
		{"2 ^ 3",1},
		{"2 << 3",16},
		{"8 >> 3",1},
		{ "-10", -10 },
		{ "-5", -5 },
		{"~2",-3},
		{ "-50 + 100 + -50", 0 },
		{ "(5 + 10 * 2 + 15 / 3) * 2 + -10", 50 },
		{ "20 + 2 * -10", 0 },
		{ "20d + 2 * -10", 0.0f },
		{ "20 + 2 * -10d", 0.0f },

	}));

	CAPTURE(input);
	REQUIRE(VmTest(input, expected));
}

TEST_CASE("Boolean Arthmetic Instructions")
{
	auto [input, expected] = GENERATE(table<std::string, ConstantValue>(
		{
			{ "true", true },
			{ "false", false },
			{ "0 < 5", true },
			{ "5 < 0", false },
			{ "0 > 5", false },
			{ "5 > 0", true },
			{ "0 <= 5", true },
			{ "5 <= 0", false },
			{ "0 >= 5", false },
			{ "5 >= 0", true },
			{ "0 == 5", false },
			{ "5 == 0", false },
			{ "0 != 5", true },
			{ "5 != 0", true },
			{ "true == true", true },
			{ "true == false", false },
			{ "true != true", false },
			{ "true != false", true },
			{ "false == false", true },
			{ "false == true", false },
			{ "false != false", false },
			{ "false != true", true },
			{ "true && true", true },
			{ "true && false", false },
			{ "false && true", false },
			{ "false && false", false },
			{ "true || true", true },
			{ "true || false", true },
			{ "false || true", true },
			{ "false || false", false },
			{ "!true", false },
			{ "!false", true },
			{ "true == true", true },
			{ "true == false", false },
			{ "true != true", false },
			{ "true != false", true },
			{ "false == false", true },
			{ "false == true", false },
			{ "false != false", false },
			{ "false != true", true },
			{ "true && true", true },
			{ "true && false", false },
			{ "false && true", false },
			{ "false && false", false },
			{ "true || true", true },
			{ "true || false", true },
			{ "false || true", true },
			{ "false || false", false },
			{ "!true", false },
			{ "!false", true },
		}));

	CAPTURE(input);
	REQUIRE(VmTest(input, expected));
}

TEST_CASE("Conditional Instructions")
{
	//these check the "last popped value" from the stack
	auto [input, expected] = GENERATE(table<std::string, ConstantValue>(
		{
			{ "if(true) {5} ", 5 },
			{ "if(true) {5} else {10} ", 5 },
			{ "if(false) {5} else {10} ", 10 },
			{ "if(1) {5} else {10} ", 5 },
			{ "if(1 < 2) {5} else {10} ", 5 },
			{ "if(1 > 2) {5} else {10} ", 10 },
			{ "if(1 > 2) {5} ", false},
			{ "if(1 < 2) {5} ", 5 },
		}));

	CAPTURE(input);
	REQUIRE(VmTest(input, expected));
}

TEST_CASE("Let assignment instructions")
{
	auto [input, expected] = GENERATE(table<std::string, ConstantValue>(
		{
			{ "let a = 5; a", 5 },
			{ "let a = 5 * 5; a", 25 },
			{ "let a = 5; let b = a; b", 5 },
			{ "let a = 5; let b = a; let c = a + b + 5; c", 15 },
		}));

	CAPTURE(input);
	REQUIRE(VmTest(input, expected));
}

TEST_CASE("Array instructions")
{
	auto [input, expected] = GENERATE(table<std::string, ConstantValue>(
		{
			{ "[1,2,3,4];", ArrayObj::New( { IntegerObj::New(1), IntegerObj::New(2), IntegerObj::New(3), IntegerObj::New(4) } ) },
			{ "[1 + 2, 3 * 4, 5 + 6];", ArrayObj::New({ IntegerObj::New(3), IntegerObj::New(12), IntegerObj::New(11) }) },
			{ "[1, 2 * 2, 3 + 3];", ArrayObj::New({ IntegerObj::New(1), IntegerObj::New(4), IntegerObj::New(6) }) },
			{ "[1, 2, 3][0]", 1 },
			{ "[1, 2, 3][1]", 2 },
			{ "[1, 2, 3][2]", 3 },
			{ "let i = 0; [1][i];", 1 },
			{ "[1, 2, 3][1 + 1];", 3 },
			{ "let myArray = [1, 2, 3]; myArray[2];", 3 },
			{ "let myArray = [1, 2, 3]; myArray[0] + myArray[1] + myArray[2];", 6 },
			{ "let myArray = [1, 2, 3]; let i = myArray[0]; myArray[i]", 2 },
		}));

	CAPTURE(input);
	REQUIRE(VmTest(input, expected));
}

TEST_CASE("Hash instructions")
{
	auto [input, expected] = GENERATE(table<std::string, ConstantValue>(
		{
			{ "{}", HashObj::New({}) },
			{ "{1: 2, 2: 3}", HashObj::New({{HashKey{ IntegerObj::New(1)->Type(),  IntegerObj::New(1)->Inspect()},HashEntry{ IntegerObj::New(1),IntegerObj::New(2)}}, {HashKey{ IntegerObj::New(2)->Type(),  IntegerObj::New(2)->Inspect()},HashEntry{ IntegerObj::New(2),IntegerObj::New(3)}}})},
			{ "{1 + 1: 2 * 2, 3 + 3: 4 * 4}", HashObj::New({{HashKey{ IntegerObj::New(2)->Type(),  IntegerObj::New(2)->Inspect()},HashEntry{ IntegerObj::New(2), IntegerObj::New(4)}}, {HashKey{ IntegerObj::New(6)->Type(),  IntegerObj::New(6)->Inspect()},HashEntry{ IntegerObj::New(6), IntegerObj::New(16)}}})},
			{ "{1: 2, 2: 3}[1]", 2 },
			{ "{1: 2, 2: 3}[2]", 3 },
			{ "{1: 2}[2]", NullObj() },
			{ "{1: 2}[0]", NullObj() },
		}));

	CAPTURE(input);
	REQUIRE(VmTest(input, expected));
}

TEST_CASE("Function instructions")
{
	auto [input, expected] = GENERATE(table<std::string, ConstantValue>(
		{
			{ "fn() { 5; }()", 5 },
			{ "fn() { let a = 5; let b = a; let c = a + b + 5; c; }()", 15 },
			{ "let a = 5; let b = 10; let c = 15; fn() { let a = 20; let b = 25; let c = 30; a + b + c; }()", 75 },
			{ "let a = 5; let b = 10; let c = 15; let test = fn() { let a = 20; let b = 25; let c = 30; a + b + c; }; test() + b;", 85 },
			{ "let identity = fn(x) { x; }; identity(5);", 5 },
			{ "fn(x) { x + 2; }(2)", 4 },
			{ "let double = fn(x) { x * 2; }; double(5);", 10 },
			{ "let add = fn(x, y) { x + y; }; add(5, 5);", 10 },
			{ "let add = fn(x, y) { x + y; }; add(5 + 5, add(5, 5));", 20 },
			{ "fn(x) { x; }(5)", 5 },
		}));

	CAPTURE(input);
	REQUIRE(VmTest(input, expected));
}

TEST_CASE("Functions without arguments")
{
	auto [input, expected] = GENERATE(table<std::string, ConstantValue>(
		{
			{ "let one = fn() { 1; }; let two = fn() {2;}; one() + two();", 3 },
			{ "let a = fn() { 1; }; let b = fn() { a() + 1; }; let c = fn() { b() + 1; }; c();", 3 },
		}));

	CAPTURE(input);
	REQUIRE(VmTest(input, expected));
}

TEST_CASE("While loop instruction")
{
	auto [input, expected] = GENERATE(table<std::string, ConstantValue>(
		{
			{"let i = 0; while (i < 10) { let i = i + 1; }; i;", 10},
			{"let i = 0; while (i < 10) { let i = i + 1; if (i == 5) { break; } }; i;", 5},
			{"let i = 0; while (i < 10) { let i = i + 1; if (i == 5) { continue; let i = 12; } }; i;", 10},
		}));

	CAPTURE(input);
	REQUIRE(VmTest(input, expected));
}

TEST_CASE("For loop instruction")
{
	auto [input, expected] = GENERATE(table<std::string, ConstantValue>(
		{
			{"let sum = 0; for (let i = 0; i < 10; i = i + 1) { sum = sum + i; }; sum;", 45},
			{"let sum = 0; for (let i = 0; i < 10; i = i + 1) { if (i == 5) { break; } sum = sum + i; }; sum;", 10},
			{"let sum = 0; for (let i = 0; i < 10; i = i + 1) { if (i == 5) { continue; } sum = sum + i; }; sum;", 40},
		}));

	CAPTURE(input);
	REQUIRE(VmTest(input, expected));
}

TEST_CASE("Extern/Builtin Function tests")
{
	auto [input, expected] = GENERATE(table<std::string, ConstantValue>(
		{
			{"len(\"\")", 0},
			{"len(\"four\")", 4},
			{"len(\"hello world\")", 11},
			{"len(1)", "argument to `len` not supported, got class IntegerObj"},
			{"len(\"one\", \"two\")", "wrong number of arguments. got=2, wanted=1"},
			{"len([1, 2, 3])", 3},
			{"first([1, 2, 3])", 1},
			{"last([1, 2, 3])", 3},
			{"rest([1, 2, 3])", ArrayObj::New({ IntegerObj::New(2), IntegerObj::New(3) })},
			{"push([1, 2, 3], 4)", ArrayObj::New({ IntegerObj::New(1), IntegerObj::New(2), IntegerObj::New(3), IntegerObj::New(4) })},
			{"push([1, 2, 3], 4, 5)", "wrong number of arguments. got=3, wanted=2"},
			{"push(1, 2)", "argument to `push` must be ARRAY, got class IntegerObj"},
		}));

	CAPTURE(input);
	REQUIRE(VmTest(input, expected));
}

TEST_CASE("Closure Tests")
{
	auto [input, expected] = GENERATE(table<std::string, ConstantValue>(
		{
			{"let newClosure = fn(a) { fn() { a; }; }; let closure = newClosure(99); closure();", 99},
			{"let newAdder = fn(a, b) { fn(c) { a + b + c; }; }; let adder = newAdder(1, 2); adder(8);", 11},
			{"let newAdder = fn(a, b) { let c = a + b; fn(d) { c + d; }; }; let adder = newAdder(1, 2); adder(8);", 11},
			{"let newClosure = fn(a) { fn(b) { a + b; }; }; let closure = newClosure(2); closure(3);", 5}
		}));

	CAPTURE(input);
	REQUIRE(VmTest(input, expected));
}

TEST_CASE("Recursive function call")
{
	auto [input, expected] = GENERATE(table<std::string, ConstantValue>(
		{
			{"let i = 0; let countDown = fn(x,y) { if (x == 0) { return y; } countDown(x - 1, y + 1); }; countDown(1,i);", 1},
			{"let i = 0; let countDown = fn(x,y) { if (x == 0) { return y; } countDown(x - 1, y + 1); }; countDown(2,i);", 2},
			{"let i = 0; let countDown = fn(x,y) { if (x == 0) { return y; } countDown(x - 1, y + 1); }; countDown(3,i);", 3},
		}));

	CAPTURE(input);
	REQUIRE(VmTest(input, expected));
}

#ifdef DO_BENCHMARK

TEST_CASE("BENCHMARK VM")
{
	auto input = "let x = 0; for (let i = 0; i < 100; i = i + 1) { x = x + i; }; x;";
	auto expected = 4950;

	SECTION("BENCHMARK VM - VERIFY")
	{
		REQUIRE(VmTest(input, expected));
	}

	SECTION("BENCHMARK VM - TIME")
	{
		BENCHMARK("BENCHMARK VM")
		{
			return VmTest(input, expected);
		};
	}
}

#endif