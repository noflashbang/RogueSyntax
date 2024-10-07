#include <vector>
#include "CompilerTestHelpers.h"
#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>


std::shared_ptr<FunctionCompiledObj> MakeFunction(Instructions instructions, int numParam, int numFree)
{
	auto fn = std::make_shared<FunctionCompiledObj>(instructions, numParam, numFree);
	return fn;
};

TEST_CASE("Instruction String")
{
	std::vector<Instructions> instructions = {
		{ OpCode::Make(OpCode::Constants::OP_CONSTANT, {1})  },
		{ OpCode::Make(OpCode::Constants::OP_CONSTANT, {2})  },
		{ OpCode::Make(OpCode::Constants::OP_CONSTANT, {65535}) }
	};

	std::string expected = { "0000:  OP_CONSTANT     1       \n0003:  OP_CONSTANT     2       \n0006:  OP_CONSTANT     65535   " };

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

	std::string expected = { "0000:  OP_ADD          \n0001:  OP_CONSTANT     2       \n0004:  OP_CONSTANT     3       " };

	auto flattened = ConcatInstructions(instructions);
	auto actual = OpCode::PrintInstructions(flattened);
	REQUIRE(actual == expected);
}

TEST_CASE("OpCode::Make tests")
{
	auto [opcode, operands, expected] = GENERATE(table<OpCode::Constants, std::vector<uint32_t>, std::vector<uint8_t>>(
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
					OpCode::Make(OpCode::Constants::OP_JUMP_IF_FALSE, { 8 }), //0001
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),        //0004
					OpCode::Make(OpCode::Constants::OP_POP, {}),              //0007
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),        //0008
					OpCode::Make(OpCode::Constants::OP_POP, {}) 		      //0011
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
					OpCode::Make(OpCode::Constants::OP_JUMP_IF_FALSE, { 11}), //0001
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),        //0004
					OpCode::Make(OpCode::Constants::OP_POP, {}),              //0007
					OpCode::Make(OpCode::Constants::OP_JUMP, { 15 }),         //0008
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),        //0011
					OpCode::Make(OpCode::Constants::OP_POP, {}),		      //0014
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),        //0015
					OpCode::Make(OpCode::Constants::OP_POP, {}) 		      //0018
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Let statement")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "let one = 1; let two = 2;", { 1, 2 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_SET, {1})
				}
			},
			{ "let one = 1; one;", { 1 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "let one = 1; let two = one; two;", { 1 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {1}),
					OpCode::Make(OpCode::Constants::OP_GET, {1}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("string tests")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{"\"1\";", {"1"},
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "\"Hello\" + \"World\";", { "Hello", "World"},
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

TEST_CASE("Array Tests")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "[]", { },
				{
					OpCode::Make(OpCode::Constants::OP_ARRAY, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "[1, 2, 3]", { 1, 2, 3 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_ARRAY, {3}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "[1 + 2, 3 - 4, 5 * 6]", { 1, 2, 3, 4, 5, 6 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {3}),
					OpCode::Make(OpCode::Constants::OP_SUB, {}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {4}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {5}),
					OpCode::Make(OpCode::Constants::OP_MUL, {}),
					OpCode::Make(OpCode::Constants::OP_ARRAY, {3}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Hash Tests")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "{}", { },
				{
					OpCode::Make(OpCode::Constants::OP_HASH, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "{1: 2, 3: 4, 5: 6}", { 1, 2, 3, 4, 5, 6 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {3}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {4}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {5}),
					OpCode::Make(OpCode::Constants::OP_HASH, {3}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "{1 + 2: 3 - 4, 5 * 6: 7 / 8}", { 1, 2, 3, 4, 5, 6, 7, 8 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {3}),
					OpCode::Make(OpCode::Constants::OP_SUB, {}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {4}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {5}),
					OpCode::Make(OpCode::Constants::OP_MUL, {}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {6}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {7}),
					OpCode::Make(OpCode::Constants::OP_DIV, {}),
					OpCode::Make(OpCode::Constants::OP_HASH, {2}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
					}
		}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Index Tests")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "[1, 2, 3][1]", { 1, 2, 3, 1 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_ARRAY, {3}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {3}),
					OpCode::Make(OpCode::Constants::OP_INDEX, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "{1: 2}[1]", { 1, 2, 1 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_HASH, {1}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_INDEX, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Function Tests")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "fn() { return 5 + 10; }", { 5, 10, MakeFunction(ConcatInstructions({
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_RETURN_VALUE, {}),
				}),0,0)},
				{
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {2, 0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "fn() { 5 + 10; }", { 5, 10, MakeFunction(ConcatInstructions({
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_RETURN_VALUE, {}),
				}),0,0)},
				{
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {2, 0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "fn() { 1; 2; }", { 1, 2, MakeFunction(ConcatInstructions({
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_RETURN_VALUE, {}),
				}),0,0)},
				{
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {2, 0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "fn() { }", { MakeFunction(ConcatInstructions({
					OpCode::Make(OpCode::Constants::OP_RETURN, {}),
				}),0,0)},
				{
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {0, 0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "fn() { 25; }()", { 25, MakeFunction(ConcatInstructions({
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_RETURN_VALUE, {}),
				}),0,0)},
				{
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {1, 0}),
					OpCode::Make(OpCode::Constants::OP_CALL, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "let noArg = fn() { 24; }; noArg();", { 24,  MakeFunction(ConcatInstructions({
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_RETURN_VALUE, {}),
				}),0,0)},
				{
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {1, 0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CALL, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "let manyArg = fn(x,y,z) { return x+y+z;}; manyArg(1,2,3);", { MakeFunction(ConcatInstructions({
				OpCode::Make(OpCode::Constants::OP_GET, {0 | 0x8000}),
				OpCode::Make(OpCode::Constants::OP_GET, {1 | 0x8000}),
				OpCode::Make(OpCode::Constants::OP_ADD, {}),
				OpCode::Make(OpCode::Constants::OP_GET, {2 | 0x8000}),
				OpCode::Make(OpCode::Constants::OP_ADD, {}),
				OpCode::Make(OpCode::Constants::OP_RETURN_VALUE, {}),
				}),0,3), 1,2,3},
				{
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {0, 0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {3}),
					OpCode::Make(OpCode::Constants::OP_CALL, {3}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Let statement scopes tests")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "let x = 5; fn() { x; }", { 5, MakeFunction(ConcatInstructions({
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_RETURN_VALUE, {}),
				}),0,0)},
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {1, 0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "fn() { let x = 5; x; }", {5, MakeFunction(ConcatInstructions({
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0 | 0x8000}),
					OpCode::Make(OpCode::Constants::OP_GET, {0 | 0x8000}),
					OpCode::Make(OpCode::Constants::OP_RETURN_VALUE, {}),
				}),1,0)},
				{
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {1,0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "fn() { let x = 5; let y = 10; x + y; }", { 5, 10, MakeFunction(ConcatInstructions({
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0 | 0x8000}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_SET, {1 | 0x8000}),
					OpCode::Make(OpCode::Constants::OP_GET, {0 | 0x8000}),
					OpCode::Make(OpCode::Constants::OP_GET, {1 | 0x8000}),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_RETURN_VALUE, {}),
				}),2,0)},
				{
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {2,0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("While test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "let x = 10; while (x > 0) { x = x - 1; }", { 10, 0, 1 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_GREATER_THAN, {}),
					OpCode::Make(OpCode::Constants::OP_JUMP_IF_FALSE, { 29 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_SUB, {}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 6 }),
				}
			},
		{ "let x = 10; while (x > 0) { x = x - 1; if(x == 5) {break;} }; x;", { 10, 0, 1, 5 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_GREATER_THAN, {}),
					OpCode::Make(OpCode::Constants::OP_JUMP_IF_FALSE, { 42 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_SUB, {}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {3}),
					OpCode::Make(OpCode::Constants::OP_EQUAL, {}),
					OpCode::Make(OpCode::Constants::OP_JUMP_IF_FALSE, { 39 }),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 42 }),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 6 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {}),
				}
		},
		{ "let x = 10; while (x > 0) { x = x - 1; if(x == 5) {continue;} }; x;", { 10, 0, 1, 5 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_GREATER_THAN, {}),
					OpCode::Make(OpCode::Constants::OP_JUMP_IF_FALSE, { 42 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_SUB, {}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {3}),
					OpCode::Make(OpCode::Constants::OP_EQUAL, {}),
					OpCode::Make(OpCode::Constants::OP_JUMP_IF_FALSE, { 39 }),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 6 }),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 6 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {}),
				}
		}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("For loop test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "for (let i = 0; i < 10; i = i + 1) { i; }", { 0, 10, 1 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_LESS_THAN, {}),
					OpCode::Make(OpCode::Constants::OP_JUMP_IF_FALSE, { 33 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 6 }),
				}
			},
			{ "for (let i = 0; i < 10; i = i + 1) { if(i == 5) {break;} }", { 0, 10, 5, 1 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_LESS_THAN, {}),
					OpCode::Make(OpCode::Constants::OP_JUMP_IF_FALSE, { 42 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_EQUAL, {}),
					OpCode::Make(OpCode::Constants::OP_JUMP_IF_FALSE, { 29 }),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 42 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {3}),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 6 }),
				}
			},
			{ "for (let i = 0; i < 10; i = i + 1) { if(i == 5) {continue;} }", { 0, 10, 5, 1 },
				{
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {1}),
					OpCode::Make(OpCode::Constants::OP_LESS_THAN, {}),
					OpCode::Make(OpCode::Constants::OP_JUMP_IF_FALSE, { 42 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_EQUAL, {}),
					OpCode::Make(OpCode::Constants::OP_JUMP_IF_FALSE, { 29 }),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 29 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {3}),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 6 }),
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("External functions")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "len([]);", { },
				{
					OpCode::Make(OpCode::Constants::OP_GET, {0 | 0x4000}),
					OpCode::Make(OpCode::Constants::OP_ARRAY, {0}),
					OpCode::Make(OpCode::Constants::OP_CALL, {1}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
		}));
	
	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Closure Test")
{
		auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<Instructions>>(
		{
			{ "let newClosure = fn(a) { fn(b) { a + b; }; }; let closure = newClosure(2); closure(3);", { 
				MakeFunction(ConcatInstructions({
					OpCode::Make(OpCode::Constants::OP_GET, {0 | 0xC000}),
					OpCode::Make(OpCode::Constants::OP_GET, {0 | 0x8000}),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_RETURN_VALUE, {}),
				}),1,1), 
				MakeFunction(ConcatInstructions({
					OpCode::Make(OpCode::Constants::OP_GET, {0 | 0x8000}),
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {0, 1}),
					OpCode::Make(OpCode::Constants::OP_RETURN_VALUE, {}),
				}),0,0), 2, 3 },
				{
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {1, 0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {2}),
					OpCode::Make(OpCode::Constants::OP_CALL, {1}),
					OpCode::Make(OpCode::Constants::OP_SET, {1}),
					OpCode::Make(OpCode::Constants::OP_GET, {1}),
					OpCode::Make(OpCode::Constants::OP_CONSTANT, {3}),
					OpCode::Make(OpCode::Constants::OP_CALL, {1}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}
