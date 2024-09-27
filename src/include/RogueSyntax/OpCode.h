#pragma once
#include <StandardLib.h>
#include <variant>

typedef std::vector<uint8_t> Instructions;

struct Definition
{
	std::string Name;
	std::vector<uint16_t> OperandWidths;
};

struct OpCode
{
	typedef uint8_t Opcode;

	enum class Constants : Opcode
	{
		//types
		OP_CONSTANT,
		OP_TRUE,
		OP_FALSE,
		OP_NULL,
		OP_ARRAY,
		OP_HASH,
		//stack
		OP_POP,
		//arithmetic
		OP_ADD,
		OP_SUB,
		OP_MUL,
		OP_DIV,
		OP_MOD,
		OP_BOR,
		OP_BAND,
		OP_BXOR,
		OP_BLSHIFT,
		OP_BRSHIFT,
		//comparison
		OP_EQUAL,
		OP_NOT_EQUAL,
		OP_GREATER_THAN,
		OP_GREATER_THAN_EQUAL,
		OP_LESS_THAN,
		OP_LESS_THAN_EQUAL,
		OP_BOOL_AND,
		OP_BOOL_OR,
		//prefix
		OP_NEGATE,
		OP_NOT,
		OP_BNOT,
		//jump
		OP_JUMP,
		OP_JUMP_IF_FALSE,
		//mem
		OP_GET_LOCAL,
		OP_SET_LOCAL,
		OP_GET_GLOBAL,
		OP_SET_GLOBAL,

	};

	static const std::unordered_map<Constants, Definition> Definitions;
	static std::variant<Definition, std::string> Lookup(const Constants opcode);
	static Instructions Make(Constants opcode, std::vector<int> operands);
	static std::tuple<Constants, std::vector<int>, size_t> ReadOperand(const Instructions& instructions, size_t offset);
	static Constants GetOpcode(const Instructions& instructions, size_t offset);
	static std::string PrintInstructions(const Instructions& instructions);

	static std::string PrintInstuctionsCompared(const Instructions& instructions, const Instructions& otherInstructions);
};


