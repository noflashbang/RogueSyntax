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
			throw std::runtime_error(std::format("Got wrong constant type. Expected={} Got={}", typeid(R).name(), typeid(*(actual.get())).name()));
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
		const auto print = OpCode::PrintInstuctionsCompared(expected, actual);
		throw std::runtime_error(std::format("Expected and actual number of instructions are not the same size. Expected={} Actual={}\n{}", expected.size(), actual.size(), print));
	}

	for (size_t i = 0; i < expected.size(); i++)
	{
		if (expected[i] != actual[i])
		{
			const auto print = OpCode::PrintInstuctionsCompared(expected, actual);
			throw std::runtime_error(std::format("Expected and actual instructions are not the same @offset{}. Expected={} Actual={}\n{}", i, expected[i], actual[i], print));
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

	std::string expected = { "0000:  OP_CONSTANT            1\n0003:  OP_CONSTANT            2\n0006:  OP_CONSTANT        65535" };

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

	std::string expected = { "0000:  OP_ADD          \n0001:  OP_CONSTANT            2\n0004:  OP_CONSTANT            3" };

	auto flattened = ConcatInstructions(instructions);
	auto actual = OpCode::PrintInstructions(flattened);
	REQUIRE(actual == expected);
}

TEST_CASE("OpCode::Make tests")
{
	auto [opcode, operands, expected] = GENERATE(table<OpCode::Constants, std::vector<int>, std::vector<uint8_t>>(
		{
			{ OpCode::Constants::OP_CONSTANT, { 65534 }, { static_cast<uint8_t>(OpCode::Constants::OP_CONSTANT) ,0xFF, 0xFE } },
			{ OpCode::Constants::OP_CONSTANT, { 65535 }, { static_cast<uint8_t>(OpCode::Constants::OP_CONSTANT) ,0xFF, 0xFF } },
			{ OpCode::Constants::OP_ADD,      { },       { static_cast<uint8_t>(OpCode::Constants::OP_ADD) } },
			{ OpCode::Constants::OP_POP,      { },       { static_cast<uint8_t>(OpCode::Constants::OP_POP) } },
			{ OpCode::Constants::OP_TRUE,     { },       { static_cast<uint8_t>(OpCode::Constants::OP_TRUE) } },
			{ OpCode::Constants::OP_FALSE,    { },       { static_cast<uint8_t>(OpCode::Constants::OP_FALSE) } },
			{ OpCode::Constants::OP_EQUAL,    { },       { static_cast<uint8_t>(OpCode::Constants::OP_EQUAL) } },
			{ OpCode::Constants::OP_NOT_EQUAL,{ },       { static_cast<uint8_t>(OpCode::Constants::OP_NOT_EQUAL) } },
			{ OpCode::Constants::OP_GREATER_THAN, { },   { static_cast<uint8_t>(OpCode::Constants::OP_GREATER_THAN) } },
			{ OpCode::Constants::OP_LESS_THAN, { },      { static_cast<uint8_t>(OpCode::Constants::OP_LESS_THAN) } },
			{ OpCode::Constants::OP_GREATER_THAN_EQUAL, { }, { static_cast<uint8_t>(OpCode::Constants::OP_GREATER_THAN_EQUAL) } },
			{ OpCode::Constants::OP_LESS_THAN_EQUAL, { }, { static_cast<uint8_t>(OpCode::Constants::OP_LESS_THAN_EQUAL) } },
			{ OpCode::Constants::OP_SUB, { }, { static_cast<uint8_t>(OpCode::Constants::OP_SUB) } },
			{ OpCode::Constants::OP_MUL, { }, { static_cast<uint8_t>(OpCode::Constants::OP_MUL) } },
			{ OpCode::Constants::OP_DIV, { }, { static_cast<uint8_t>(OpCode::Constants::OP_DIV) } },
			{ OpCode::Constants::OP_MOD, { }, { static_cast<uint8_t>(OpCode::Constants::OP_MOD) } },
			{ OpCode::Constants::OP_BOR, { }, { static_cast<uint8_t>(OpCode::Constants::OP_BOR) } },
			{ OpCode::Constants::OP_BAND, { }, { static_cast<uint8_t>(OpCode::Constants::OP_BAND) } },
			{ OpCode::Constants::OP_BXOR, { }, { static_cast<uint8_t>(OpCode::Constants::OP_BXOR) } },
			{ OpCode::Constants::OP_BLSHIFT, { }, { static_cast<uint8_t>(OpCode::Constants::OP_BLSHIFT) } },
			{ OpCode::Constants::OP_BRSHIFT, { }, { static_cast<uint8_t>(OpCode::Constants::OP_BRSHIFT) } }
		}));

	CAPTURE(expected);
	auto instructions = OpCode::Make(opcode, operands);
	REQUIRE(instructions == expected);
}

TEST_CASE("Expression Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1; 2", { 1, 2 }, 
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_POP, {}) 
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Greaterthan Comparison Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 > 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_GREATER_THAN, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("LessThan Comparison Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 < 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_LESS_THAN, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Equal Comparison Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 == 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_EQUAL, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("NotEqual Comparison Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 != 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_NOT_EQUAL, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("GreaterThanOrEqual Comparison Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 >= 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_GREATER_THAN_EQUAL, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("LessThanOrEqual Comparison Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 <= 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_LESS_THAN_EQUAL, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Addition Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 + 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Addition 2 Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 + 2 + 3 + 4 + 5", { 1, 2, 3, 4, 5 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {3}),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {4}),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}


TEST_CASE("Subtraction Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 - 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_SUB, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Multiplication Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 * 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_MUL, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Division Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 / 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_DIV, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Modulus Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 % 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_MOD, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Bitwise OR Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 | 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_BOR, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Bitwise AND Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 & 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_BAND, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Bitwise XOR Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 ^ 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_BXOR, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Bitwise Left Shift Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"1 << 2", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_BLSHIFT, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Bitwise Right Shift Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"8 >> 2", { 8, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_BRSHIFT, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Negate test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "-1", { 1 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_NEGATE, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Not test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "!true", { },
				{
					OpCode::Make(OpCode::Constants::OP_TRUE, {}),
					OpCode::Make(OpCode::Constants::OP_NOT, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Boolean AND test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "true && false", { },
				{
					OpCode::Make(OpCode::Constants::OP_TRUE, {}),
					OpCode::Make(OpCode::Constants::OP_FALSE, {}),
					OpCode::Make(OpCode::Constants::OP_BOOL_AND, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Boolean OR test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "true || false", { },
				{
					OpCode::Make(OpCode::Constants::OP_TRUE, {}),
					OpCode::Make(OpCode::Constants::OP_FALSE, {}),
					OpCode::Make(OpCode::Constants::OP_BOOL_OR, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("BNOT test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "~5", { 5 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_BNOT, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Conditional Test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "if (true) { 10 }; 3333;", { 10, 3333 },
				{
					OpCode::Make(OpCode::Constants::OP_TRUE, {}),             //0000
					OpCode::Make(OpCode::Constants::OP_JUMP_IF_FALSE, { 10 }),//0001
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),        //0004
					OpCode::Make(OpCode::Constants::OP_JUMP, {11}),           //0007
					OpCode::Make(OpCode::Constants::OP_NULL, {}),             //0010
					OpCode::Make(OpCode::Constants::OP_POP, {}),              //0011
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),        //0012
					OpCode::Make(OpCode::Constants::OP_POP, {}) 		      //0015
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Conditional with Else Test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "if (true) { 10 } else { 20 }; 3333;", { 10, 20, 3333 },
				{
					OpCode::Make(OpCode::Constants::OP_TRUE, {}),             //0000
					OpCode::Make(OpCode::Constants::OP_JUMP_IF_FALSE, { 10}), //0001
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),        //0004
					OpCode::Make(OpCode::Constants::OP_JUMP, { 13 }),         //0007
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),        //0010
					OpCode::Make(OpCode::Constants::OP_POP, {}),		      //0013
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),        //0014
					OpCode::Make(OpCode::Constants::OP_POP, {}) 		      //0017
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}