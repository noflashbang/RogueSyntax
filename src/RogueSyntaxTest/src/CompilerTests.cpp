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

bool TestConstants(const std::vector<ConstantValue>& expected, const std::vector<std::shared_ptr<IObject>>& actual)
{
	if (expected.size() != actual.size())
	{
		throw std::runtime_error(std::format("Expected and actual number of constants are not the same size. Expected={} Actual={}",expected.size(), actual.size()));
	}

	for (size_t i = 0; i < expected.size(); i++)
	{
		auto expectedValue = expected[i];
		auto actualValue = actual[i];
		if (std::holds_alternative<int>(expectedValue))
		{
			return TestConstantValues<int, IntegerObj>(expectedValue, actualValue);
		}
		else if (std::holds_alternative<std::string>(expectedValue))
		{
			return TestConstantValues<std::string, StringObj>(expectedValue, actualValue);
		}
		else if (std::holds_alternative<float>(expectedValue))
		{
			return TestConstantValues<float, DecimalObj>(expectedValue, actualValue);
		}
		else if (std::holds_alternative<bool>(expectedValue))
		{
			return TestConstantValues<bool, BooleanObj>(expectedValue, actualValue);
		}
		else
		{
			throw std::runtime_error(std::format("Unknown constant type."));
		}
	}
	return true;
}

bool TestInstructions(const Instructions& expected, const Instructions& actual)
{
	if (expected.size() != actual.size())
	{
		throw std::runtime_error(std::format("Expected and actual number of instructions are not the same size. Expected={} Actual={}", expected.size(), actual.size()));
	}

	for (size_t i = 0; i < expected.size(); i++)
	{
		if (expected[i] != actual[i])
		{
			throw std::runtime_error(std::format("Expected and actual instructions are not the same @offset{}. Expected={} Actual={}", i, expected[i], actual[i]));
		}
	}
	return true;
}

Instructions ConcatInstructions(const std::vector<Instructions>& instructions)
{
	Instructions result;
	for (const auto& instruction : instructions)
	{
		result.insert(result.end(), instruction.begin(), instruction.end());
	}
	return result;
}

bool TestByteCode(const std::vector<ConstantValue>& expectedConstants, const std::vector<Instructions>& expectedInstructions, const ByteCode& actual)
{
	auto flattened = ConcatInstructions(expectedInstructions);
	return TestInstructions(flattened, actual.Instructions) && TestConstants(expectedConstants, actual.Constants);
}

bool CompilerTest(const std::vector<ConstantValue>& expectedConstants, const std::vector<Instructions>& expectedInstructions, std::string input)
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
	return TestByteCode(expectedConstants, expectedInstructions, byteCode);
}

TEST_CASE("Instruction String")
{
	std::vector<Instructions> instructions = {
		{ OpCode::Make(OpCode::Constants::OP_CONSTANT, {1})  },
		{ OpCode::Make(OpCode::Constants::OP_CONSTANT, {2})  },
		{ OpCode::Make(OpCode::Constants::OP_CONSTANT, {65535}) }
	};

	std::string expected = { "0000:  OP_CONSTANT            1\n0003:  OP_CONSTANT            2\n0006:  OP_CONSTANT        65535\n" };

	auto flattened = ConcatInstructions(instructions);
	auto actual = OpCode::PrintInstructions(flattened);
	REQUIRE(actual == expected);
}

TEST_CASE("Instruction String 2")
{
	std::vector<Instructions> instructions = {
		{ OpCode::Make(OpCode::Constants::OP_ADD, {})  },
		{ OpCode::Make(OpCode::Constants::OP_CONSTANT, {2})  },
		{ OpCode::Make(OpCode::Constants::OP_CONSTANT, {3}) }
	};

	std::string expected = { "0000:  OP_ADD          \n0001:  OP_CONSTANT            2\n0004:  OP_CONSTANT            3\n" };

	auto flattened = ConcatInstructions(instructions);
	auto actual = OpCode::PrintInstructions(flattened);
	REQUIRE(actual == expected);
}

TEST_CASE("Simple Instructions")
{
	auto [opcode, operands, expected] = GENERATE(table<OpCode::Constants, std::vector<int>, std::vector<uint8_t>>(
		{
			{ OpCode::Constants::OP_CONSTANT, { 65534 }, { static_cast<uint8_t>(OpCode::Constants::OP_CONSTANT) ,0xFF, 0xFE } },
			{ OpCode::Constants::OP_CONSTANT, { 65535 }, { static_cast<uint8_t>(OpCode::Constants::OP_CONSTANT) ,0xFF, 0xFF } },
			{ OpCode::Constants::OP_ADD,      { },       { static_cast<uint8_t>(OpCode::Constants::OP_ADD) } }
		}));

	CAPTURE(expected);
	auto instructions = OpCode::Make(opcode, operands);
	REQUIRE(instructions == expected);
}

TEST_CASE("Complier")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1+2", { 1, 2 }, {OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}), OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}), OpCode::Make(OpCode::Constants::OP_ADD, {})}}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

