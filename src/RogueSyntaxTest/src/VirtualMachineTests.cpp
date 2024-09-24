#include <vector>
#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

typedef std::variant<int, std::string, float, bool> ConstantValue;

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
			throw std::runtime_error(std::format("Got wrong constant type. Expected={} Got={}", typeid(T).name(), typeid(R).name()));
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
	else
	{
		throw std::runtime_error(std::format("Unknown constant type."));
	}
	return true;
}

bool VmTest(std::string input, int32_t expected)
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

	auto actual = vm.GetTop();
	REQUIRE(actual != nullptr);
	REQUIRE(actual->Type() == ObjectType::INTEGER_OBJ);
	auto integer = std::dynamic_pointer_cast<IntegerObj>(actual);
	REQUIRE(integer != nullptr);
	REQUIRE(integer->Value == expected);

	return true;
}

TEST_CASE("Integer Arthmetic Instructions")
{
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
	REQUIRE(VmTest(input, expected));
}