#include <vector>
#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

typedef std::variant<int, std::string, float, bool, NullObj, std::shared_ptr<ArrayObj>, std::shared_ptr<HashObj>> ConstantValue;

bool TestIObjects(std::shared_ptr<IObject> expected, std::shared_ptr<IObject> actual)
{
	if (typeid(*(expected.get())) != typeid(*(actual.get())))
	{
		throw std::runtime_error(std::format("Expected and actual object types are not the same. Expected={} Actual={}", typeid(*(expected.get())).name(), typeid(*(actual.get())).name()));
	}
	if (expected->Inspect() != actual->Inspect())
	{
		throw std::runtime_error(std::format("Expected and actual object values are not the same. Expected={} Actual={}", expected->Inspect(), actual->Inspect()));
	}
	return true;
}

template<typename T, typename R>
bool TestConstantValues(ConstantValue expected, std::shared_ptr<IObject> actual)
{
	if (std::holds_alternative<T>(expected))
	{
		if (typeid(*(actual.get())) == typeid(R))
		{
			if (std::get<T>(expected) != std::dynamic_pointer_cast<R>(actual)->Value)
			{
				throw std::runtime_error(std::format("Expected and actual constant values are not the same. Expected={} Actual={}", std::get<T>(expected), std::dynamic_pointer_cast<R>(actual)->Value));
			}
		}
		else
		{
			throw std::runtime_error(std::format("Got wrong constant type. Expected={} Got={}", typeid(R).name(), typeid(*(actual.get())).name()));
		}
	}
	return true;
}

bool TestConstant(const ConstantValue& expected, const std::shared_ptr<IObject>& actual)
{
	if (std::holds_alternative<int>(expected))
	{
		return TestConstantValues<int, IntegerObj>(expected, actual);
	}
	else if (std::holds_alternative<std::string>(expected))
	{
		return TestConstantValues<std::string, StringObj>(expected, actual);
	}
	else if (std::holds_alternative<float>(expected))
	{
		return TestConstantValues<float, DecimalObj>(expected, actual);
	}
	else if (std::holds_alternative<bool>(expected))
	{
		return TestConstantValues<bool, BooleanObj>(expected, actual);
	}
	else if (std::holds_alternative<NullObj>(expected))
	{
		if (typeid(*(actual.get())) != typeid(NullObj))
		{
			throw std::runtime_error(std::format("Expected and actual constant values are not the same. Expected={} Actual={}", "null", actual->Inspect()));
		}
	}
	else if (std::holds_alternative<std::shared_ptr<ArrayObj>>(expected))
	{
		auto arr = std::get<std::shared_ptr<ArrayObj>>(expected);
		auto actualArr = std::dynamic_pointer_cast<ArrayObj>(actual);

		for (int i = 0; i < arr->Elements.size(); i++)
		{
			auto expectedValue = arr->Elements[i];
			auto actualValue = actualArr->Elements[i];
			TestIObjects(expectedValue, actualValue);
		}
	}
	else if (std::holds_alternative<std::shared_ptr<HashObj>>(expected))
	{
		auto hash = std::get<std::shared_ptr<HashObj>>(expected);
		auto actualHash = std::dynamic_pointer_cast<HashObj>(actual);

		for (const auto& [key, value] : hash->Elements)
		{
			auto actualValue = actualHash->Elements[key];
			TestIObjects(value.Value, actualValue.Value);
		}
	}
	else
	{
		throw std::runtime_error(std::format("Unknown constant type."));
	}
	return true;
}

bool VmTest(std::string input, ConstantValue expected)
{
	Compiler compiler;
	Lexer lexer(input);
	Parser parser(lexer);

	auto node = parser.ParseProgram();
	auto errors = parser.Errors();
	if (errors.size() > 0)
	{
		for (const auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	compiler.Compile(node);
	errors = compiler.GetErrors();
	if (errors.size() > 0)
	{
		for (const auto& error : errors)
		{
			UNSCOPED_INFO(error);
		}
	}
	REQUIRE(errors.size() == 0);

	auto byteCode = compiler.GetByteCode();
	RogueVM vm(byteCode);
	vm.Run();

	auto actual = vm.LastPoppped();
	REQUIRE(actual != nullptr);
	return TestConstant(expected, actual);
}

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
	auto [input, expected] = GENERATE(table<std::string, ConstantValue>(
		{
			{ "if(true) {5} ", 5 },
			{ "if(true) {5} else {10} ", 5 },
			{ "if(false) {5} else {10} ", 10 },
			{ "if(1) {5} else {10} ", 5 },
			{ "if(1 < 2) {5} else {10} ", 5 },
			{ "if(1 > 2) {5} else {10} ", 10 },
			{ "if(1 > 2) {5} ", NullObj()},
			{ "if(1 < 2) {5} ", 5 },
			{ "if(if(false) {10}) {10} else {20}", 20},
			{ "if(if(true) {10}) {10} else {20}", 10},
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
			//{ "[1, 2, 3][0]", 1 },
			//{ "[1, 2, 3][1]", 2 },
			//{ "[1, 2, 3][2]", 3 },
			//{ "let i = 0; [1][i];", 1 },
			//{ "[1, 2, 3][1 + 1];", 3 },
			//{ "let myArray = [1, 2, 3]; myArray[2];", 3 },
			//{ "let myArray = [1, 2, 3]; myArray[0] + myArray[1] + myArray[2];", 6 },
			//{ "let myArray = [1, 2, 3]; let i = myArray[0]; myArray[i]", 2 },
			//{ "[1, 2, 3][3]", NullObj() },
			//{ "[1, 2, 3][-1]", NullObj() },
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
			//{ "{1: 2, 2: 3}[1]", 2 },
			//{ "{1: 2, 2: 3}[2]", 3 },
			//{ "{1: 2}[2]", NullObj() },
			//{ "{1: 2}[0]", NullObj() },
		}));

	CAPTURE(input);
	REQUIRE(VmTest(input, expected));
}
