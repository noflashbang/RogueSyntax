#include <vector>
#include "CompilerTestHelpers.h"
#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>


std::shared_ptr<FunctionCompiledObj> MakeFunction(RSInstructions instructions, int numLocals, int numParameters)
{
	auto fn = std::make_shared<FunctionCompiledObj>(instructions, numLocals, numParameters);
	return fn;
};

RSInstructions MakeFunctionLiteral(const FunctionCompiledObj* pFun)
{
	uint32_t numLocals = pFun->NumLocals;
	uint32_t numParameters = pFun->NumParameters;
	uint32_t numInstructions = pFun->FuncInstructions.size();
	auto instructions = pFun->FuncInstructions;
	return OpCode::Make(OpCode::Constants::OP_LFUN, { numLocals, numParameters, numInstructions }, instructions);
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
			{ OpCode::Constants::OP_EQ,    { },       { static_cast<uint8_t>(OpCode::Constants::OP_EQ) } },
			{ OpCode::Constants::OP_NEQ,{ },       { static_cast<uint8_t>(OpCode::Constants::OP_NEQ) } },
			{ OpCode::Constants::OP_GT, { },   { static_cast<uint8_t>(OpCode::Constants::OP_GT) } },
			{ OpCode::Constants::OP_LT, { },      { static_cast<uint8_t>(OpCode::Constants::OP_LT) } },
			{ OpCode::Constants::OP_GTE, { }, { static_cast<uint8_t>(OpCode::Constants::OP_GTE) } },
			{ OpCode::Constants::OP_LTE, { }, { static_cast<uint8_t>(OpCode::Constants::OP_LTE) } },
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1; 2", { }, 
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::Make(OpCode::Constants::OP_POP, {}),
					OpCode::MakeIntegerLiteral(2),
					OpCode::Make(OpCode::Constants::OP_POP, {}) 
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Greaterthan Comparison Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 > 2", { },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
					OpCode::Make(OpCode::Constants::OP_GT, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("LessThan Comparison Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 < 2", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
					OpCode::Make(OpCode::Constants::OP_LT, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Equal Comparison Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 == 2", {},
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
					OpCode::Make(OpCode::Constants::OP_EQ, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("NotEqual Comparison Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 != 2", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
					OpCode::Make(OpCode::Constants::OP_NEQ, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("GreaterThanOrEqual Comparison Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 >= 2", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
					OpCode::Make(OpCode::Constants::OP_GTE, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("LessThanOrEqual Comparison Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 <= 2", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
					OpCode::Make(OpCode::Constants::OP_LTE, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Addition Complier test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 + 2", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 + 2 + 3 + 4 + 5", { },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::MakeIntegerLiteral(3),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::MakeIntegerLiteral(4),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::MakeIntegerLiteral(5),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 - 2", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 * 2", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 / 2", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 % 2", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 | 2", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 & 2", { },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 ^ 2", { },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"1 << 2", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"8 >> 2", { },
				{
					OpCode::MakeIntegerLiteral(8),
					OpCode::MakeIntegerLiteral(2),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "-1", {  },
				{
					OpCode::MakeIntegerLiteral(1),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "true && false", { },
				{
					OpCode::Make(OpCode::Constants::OP_TRUE, {}),
					OpCode::Make(OpCode::Constants::OP_FALSE, {}),
					OpCode::Make(OpCode::Constants::OP_AND, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Boolean OR test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "true || false", { },
				{
					OpCode::Make(OpCode::Constants::OP_TRUE, {}),
					OpCode::Make(OpCode::Constants::OP_FALSE, {}),
					OpCode::Make(OpCode::Constants::OP_OR, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("BNOT test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "~5", {  },
				{
					OpCode::MakeIntegerLiteral(5),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "if (true) { 10 }; 3333;", { },
				{
					OpCode::Make(OpCode::Constants::OP_TRUE, {}),             //0000
					OpCode::Make(OpCode::Constants::OP_JUMPIFZ, { 10 }), //0001
					OpCode::MakeIntegerLiteral(10),                           //0004
					OpCode::Make(OpCode::Constants::OP_POP, {}),              //0007
					OpCode::MakeIntegerLiteral(3333),                         //0008
					OpCode::Make(OpCode::Constants::OP_POP, {}) 		      //0011
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Conditional with Else Test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "if (true) { 10 } else { 20 }; 3333;", {  },
				{
					OpCode::Make(OpCode::Constants::OP_TRUE, {}),             //0000
					OpCode::Make(OpCode::Constants::OP_JUMPIFZ, { 13}), //0001
					OpCode::MakeIntegerLiteral(10),                           //0004
					OpCode::Make(OpCode::Constants::OP_POP, {}),              //0007
					OpCode::Make(OpCode::Constants::OP_JUMP, { 19 }),         //0008
					OpCode::MakeIntegerLiteral(20),                           //0011
					OpCode::Make(OpCode::Constants::OP_POP, {}),		      //0014
					OpCode::MakeIntegerLiteral(3333),                         //0015
					OpCode::Make(OpCode::Constants::OP_POP, {}) 		      //0018
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Let statement")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "let one = 1; let two = 2;", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::MakeIntegerLiteral(2),
					OpCode::Make(OpCode::Constants::OP_SET, {1})
				}
			},
			{ "let one = 1; one;", { },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "let one = 1; let two = one; two;", {  },
				{
					OpCode::MakeIntegerLiteral(1),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{"\"1\";", {},
				{
					OpCode::MakeStringLiteral("1"),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "\"Hello\" + \"World\";", { },
				{
					OpCode::MakeStringLiteral("Hello"),
					OpCode::MakeStringLiteral("World"),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "[]", { },
				{
					OpCode::Make(OpCode::Constants::OP_ARRAY, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "[1, 2, 3]", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
					OpCode::MakeIntegerLiteral(3),
					OpCode::Make(OpCode::Constants::OP_ARRAY, {3}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "[1 + 2, 3 - 4, 5 * 6]", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::MakeIntegerLiteral(3),
					OpCode::MakeIntegerLiteral(4),
					OpCode::Make(OpCode::Constants::OP_SUB, {}),
					OpCode::MakeIntegerLiteral(5),
					OpCode::MakeIntegerLiteral(6),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "{}", { },
				{
					OpCode::Make(OpCode::Constants::OP_HASH, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "{1: 2, 3: 4, 5: 6}", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
					OpCode::MakeIntegerLiteral(3),
					OpCode::MakeIntegerLiteral(4),
					OpCode::MakeIntegerLiteral(5),
					OpCode::MakeIntegerLiteral(6),
					OpCode::Make(OpCode::Constants::OP_HASH, {3}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "{1 + 2: 3 - 4}", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::MakeIntegerLiteral(3),
					OpCode::MakeIntegerLiteral(4),
					OpCode::Make(OpCode::Constants::OP_SUB, {}),
					OpCode::Make(OpCode::Constants::OP_HASH, {1}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
					}
		}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("Index Tests")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "[1, 2, 3][1]", { },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
					OpCode::MakeIntegerLiteral(3),
					OpCode::Make(OpCode::Constants::OP_ARRAY, {3}),
					OpCode::MakeIntegerLiteral(1),
					OpCode::Make(OpCode::Constants::OP_INDEX, {}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "{1: 2}[1]", {  },
				{
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
					OpCode::Make(OpCode::Constants::OP_HASH, {1}),
					OpCode::MakeIntegerLiteral(1),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "fn() { return 5 + 10; }", { },
				{
					MakeFunctionLiteral
					(
						MakeFunction
						(
							ConcatInstructions
							(
								{
									OpCode::MakeIntegerLiteral(5),
									OpCode::MakeIntegerLiteral(10),
									OpCode::Make(OpCode::Constants::OP_ADD, {}),
									OpCode::Make(OpCode::Constants::OP_RET_VAL, {}),
								}
							),0,0
						).get()
					),
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "fn() { 5 + 10; }", { },
				{
					MakeFunctionLiteral
					(
						MakeFunction
						(
							ConcatInstructions
							(
								{
									OpCode::MakeIntegerLiteral(5),
									OpCode::MakeIntegerLiteral(10),
									OpCode::Make(OpCode::Constants::OP_ADD, {}),
									OpCode::Make(OpCode::Constants::OP_RET_VAL, {}),
								}
							),0,0
						).get()
					),
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},

			{ "fn() { 1; 2; }", { },
				{
					MakeFunctionLiteral
					(
						MakeFunction
						(
							ConcatInstructions
							(
								{
									OpCode::MakeIntegerLiteral(1),
									OpCode::Make(OpCode::Constants::OP_POP, {}),
									OpCode::MakeIntegerLiteral(2),
									OpCode::Make(OpCode::Constants::OP_RET_VAL, {}),
								}
							),0,0
						).get()
					),
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "fn() { }", { },
				{
					MakeFunctionLiteral
					(
						MakeFunction
						(
							ConcatInstructions
							(
								{
									OpCode::Make(OpCode::Constants::OP_RETURN, {}),
								}
							),0,0
						).get()
					),
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "fn() { 25; }()", {  },
				{
					MakeFunctionLiteral
					(
						MakeFunction
						(
							ConcatInstructions
							(
								{
									OpCode::MakeIntegerLiteral(25),
									OpCode::Make(OpCode::Constants::OP_RET_VAL, {}),
								}
							),0,0
						).get()
					),
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {0}),
					OpCode::Make(OpCode::Constants::OP_CALL, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "let noArg = fn() { 24; }; noArg();", { },
				{
					MakeFunctionLiteral
					(
						MakeFunction
						(
							ConcatInstructions
							(
								{
									OpCode::MakeIntegerLiteral(24),
									OpCode::Make(OpCode::Constants::OP_RET_VAL, {}),
								}
							),1,0
						).get()
					),
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_CALL, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "let manyArg = fn(x,y,z) { return x+y+z;}; manyArg(1,2,3);", { },
				{
					MakeFunctionLiteral
					(
						MakeFunction
						(
							ConcatInstructions
							(
								{
									OpCode::Make(OpCode::Constants::OP_GET, {0 | 0x8000}),
									OpCode::Make(OpCode::Constants::OP_GET, {1 | 0x8000}),
									OpCode::Make(OpCode::Constants::OP_ADD, {}),
									OpCode::Make(OpCode::Constants::OP_GET, {2 | 0x8000}),
									OpCode::Make(OpCode::Constants::OP_ADD, {}),
									OpCode::Make(OpCode::Constants::OP_RET_VAL, {}),
								}
							),4,3
						).get()
					),
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(1),
					OpCode::MakeIntegerLiteral(2),
					OpCode::MakeIntegerLiteral(3),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "let x = 5; fn() { x; }", { },
				{
					OpCode::MakeIntegerLiteral(5),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					MakeFunctionLiteral
					(
						MakeFunction
						(
							ConcatInstructions
							(
								{
									OpCode::Make(OpCode::Constants::OP_GET, {0}),
									OpCode::Make(OpCode::Constants::OP_RET_VAL, {}),
								}
							),0,0
						).get()
					),
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "fn() { let x = 5; x; }", {},
				{
					MakeFunctionLiteral
					(
						MakeFunction
						(
							ConcatInstructions
							(
								{
									OpCode::MakeIntegerLiteral(5),
									OpCode::Make(OpCode::Constants::OP_SET, {0 | 0x8000}),
									OpCode::Make(OpCode::Constants::OP_GET, {0 | 0x8000}),
									OpCode::Make(OpCode::Constants::OP_RET_VAL, {}),
								}
							),1,0
						).get()
					),
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
			{ "fn() { let x = 5; let y = 10; x + y; }", {},
				{
					MakeFunctionLiteral
					(
						MakeFunction
						(
							ConcatInstructions
							(
								{
									OpCode::MakeIntegerLiteral(5),
									OpCode::Make(OpCode::Constants::OP_SET, {0 | 0x8000}),
									OpCode::MakeIntegerLiteral(10),
									OpCode::Make(OpCode::Constants::OP_SET, {1 | 0x8000}),
									OpCode::Make(OpCode::Constants::OP_GET, {0 | 0x8000}),
									OpCode::Make(OpCode::Constants::OP_GET, {1 | 0x8000}),
									OpCode::Make(OpCode::Constants::OP_ADD, {}),
									OpCode::Make(OpCode::Constants::OP_RET_VAL, {}),
								}
							),2,0
						).get()
					),
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("While test")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "let x = 10; while (x > 0) { x = x - 1; }", {  },
				{
					OpCode::MakeIntegerLiteral(10),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(0),
					OpCode::Make(OpCode::Constants::OP_GT, {}),
					OpCode::Make(OpCode::Constants::OP_JUMPIFZ, { 35 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(1),
					OpCode::Make(OpCode::Constants::OP_SUB, {}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 8 }),
				}
			},
		{ "let x = 10; while (x > 0) { x = x - 1; if(x == 5) {break;} }; x;", { },
				{
					OpCode::MakeIntegerLiteral(10),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(0),
					OpCode::Make(OpCode::Constants::OP_GT, {}),
					OpCode::Make(OpCode::Constants::OP_JUMPIFZ, { 50 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(1),
					OpCode::Make(OpCode::Constants::OP_SUB, {}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(5),
					OpCode::Make(OpCode::Constants::OP_EQ, {}),
					OpCode::Make(OpCode::Constants::OP_JUMPIFZ, { 47 }),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 50 }),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 8 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {}),
				}
		},
		{ "let x = 10; while (x > 0) { x = x - 1; if(x == 5) {continue;} }; x;", {  },
				{
					OpCode::MakeIntegerLiteral(10),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(0),
					OpCode::Make(OpCode::Constants::OP_GT, {}),
					OpCode::Make(OpCode::Constants::OP_JUMPIFZ, { 50 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(1),
					OpCode::Make(OpCode::Constants::OP_SUB, {}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(5),
					OpCode::Make(OpCode::Constants::OP_EQ, {}),
					OpCode::Make(OpCode::Constants::OP_JUMPIFZ, { 47 }),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 8 }),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 8 }),
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
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "for (let i = 0; i < 10; i = i + 1) { i; }", {  },
				{
					OpCode::MakeIntegerLiteral(0),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(10),
					OpCode::Make(OpCode::Constants::OP_LT, {}),
					OpCode::Make(OpCode::Constants::OP_JUMPIFZ, { 39 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::Make(OpCode::Constants::OP_POP, {}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(1),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 8 }),
				}
			},
			{ "for (let i = 0; i < 10; i = i + 1) { if(i == 5) {break;} }", { },
				{
					OpCode::MakeIntegerLiteral(0),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(10),
					OpCode::Make(OpCode::Constants::OP_LT, {}),
					OpCode::Make(OpCode::Constants::OP_JUMPIFZ, { 50 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(5),
					OpCode::Make(OpCode::Constants::OP_EQ, {}),
					OpCode::Make(OpCode::Constants::OP_JUMPIFZ, { 35 }),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 50 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(1),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 8 }),
				}
			},
			{ "for (let i = 0; i < 10; i = i + 1) { if(i == 5) {continue;} }", {  },
				{
					OpCode::MakeIntegerLiteral(0),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(10),
					OpCode::Make(OpCode::Constants::OP_LT, {}),
					OpCode::Make(OpCode::Constants::OP_JUMPIFZ, { 50 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(5),
					OpCode::Make(OpCode::Constants::OP_EQ, {}),
					OpCode::Make(OpCode::Constants::OP_JUMPIFZ, { 35 }),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 35 }),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(1),
					OpCode::Make(OpCode::Constants::OP_ADD, {}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_JUMP, { 8 }),
				}
			}
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}

TEST_CASE("External functions")
{
	auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
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
		auto [input, expectedConstants, expectedInstructions] = GENERATE(table<std::string, std::vector<ConstantValue>, std::vector<RSInstructions>>(
		{
			{ "let newClosure = fn(a) { fn(b) { a + b; }; }; let closure = newClosure(2); closure(3);", { },
				{
					MakeFunctionLiteral
					(
						MakeFunction
						(
							ConcatInstructions
							(
								{
									OpCode::Make(OpCode::Constants::OP_GET, {0 | 0x8000}),
									MakeFunctionLiteral
									(
										MakeFunction
										(
											ConcatInstructions
											(
												{
													OpCode::Make(OpCode::Constants::OP_GET, {0 | 0xC000}),
													OpCode::Make(OpCode::Constants::OP_GET, {0 | 0x8000}),
													OpCode::Make(OpCode::Constants::OP_ADD, {}),
													OpCode::Make(OpCode::Constants::OP_RET_VAL, {}),
												}
											),2,1
										).get()
									),
									OpCode::Make(OpCode::Constants::OP_CLOSURE, {1}),
									OpCode::Make(OpCode::Constants::OP_RET_VAL, {}),
								}
							),2,1
						).get()
					),
					OpCode::Make(OpCode::Constants::OP_CLOSURE, {0}),
					OpCode::Make(OpCode::Constants::OP_SET, {0}),
					OpCode::Make(OpCode::Constants::OP_GET, {0}),
					OpCode::MakeIntegerLiteral(2),
					OpCode::Make(OpCode::Constants::OP_CALL, {1}),
					OpCode::Make(OpCode::Constants::OP_SET, {1}),
					OpCode::Make(OpCode::Constants::OP_GET, {1}),
					OpCode::MakeIntegerLiteral(3),
					OpCode::Make(OpCode::Constants::OP_CALL, {1}),
					OpCode::Make(OpCode::Constants::OP_POP, {})
				}
			},
		}));

	CAPTURE(input);
	REQUIRE(CompilerTest(expectedConstants, expectedInstructions, input));
}
