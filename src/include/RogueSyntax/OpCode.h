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
		OP_CONSTANT,
		OP_POP,
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
	};

	static const std::unordered_map<Constants, Definition> Definitions;
	static std::variant<Definition, std::string> Lookup(const Constants opcode);
	static Instructions Make(Constants opcode, std::vector<int> operands);
	static std::tuple<Constants, std::vector<int>, size_t> ReadOperand(const Instructions& instructions, size_t offset);
	static Constants GetOpcode(const Instructions& instructions, size_t offset);
	static std::string PrintInstructions(const Instructions& instructions);
};


