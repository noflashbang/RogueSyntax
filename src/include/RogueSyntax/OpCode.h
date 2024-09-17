#pragma once
#include <StandardLib.h>
#include <variant>

typedef uint8_t Opcode;

typedef std::vector<uint8_t> Instructions;

enum class OpCodeConstants : Opcode
{
	OP_CONSTANT,
};

struct Definition
{
	std::string Name;
	std::vector<uint16_t> OperandWidths;
};

struct StdOpCode
{
	static const std::unordered_map<OpCodeConstants, Definition> OpCodeDefinitions;
	static std::variant<Definition, std::string> Lookup(const OpCodeConstants opcode);
	static Instructions Make(OpCodeConstants opcode, std::vector<int> operands);
	static std::tuple<OpCodeConstants, std::vector<int>, size_t> ReadOperand(const Instructions& instructions, size_t offset);
	static std::string PrintInstructions(const Instructions& instructions);
};


