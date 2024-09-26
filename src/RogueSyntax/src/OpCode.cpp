#include "OpCode.h"
#include <pch.h>

const std::unordered_map<OpCode::Constants, Definition> OpCode::Definitions = {
	{ OpCode::Constants::OP_CONSTANT, Definition{ "OP_CONSTANT", { 2 } } },
	{ OpCode::Constants::OP_FALSE,    Definition{ "OP_FALSE", {} } },
	{ OpCode::Constants::OP_TRUE,     Definition{ "OP_TRUE", {} } },
	{ OpCode::Constants::OP_NULL,     Definition{ "OP_NULL", {} } },
	{ OpCode::Constants::OP_POP,      Definition{ "OP_POP", {} } },
	{ OpCode::Constants::OP_ADD,      Definition{ "OP_ADD", {} } },
	{ OpCode::Constants::OP_SUB,      Definition{ "OP_SUB", {} } },
	{ OpCode::Constants::OP_MUL,      Definition{ "OP_MUL", {} } },
	{ OpCode::Constants::OP_DIV,      Definition{ "OP_DIV", {} } },
	{ OpCode::Constants::OP_MOD,      Definition{ "OP_MOD", {} } },
	{ OpCode::Constants::OP_BOR,      Definition{ "OP_BOR", {} } },
	{ OpCode::Constants::OP_BAND,     Definition{ "OP_BAND", {} } },
	{ OpCode::Constants::OP_BXOR,     Definition{ "OP_BXOR", {} } },
	{ OpCode::Constants::OP_BLSHIFT,  Definition{ "OP_BLSHIFT", {} } },
	{ OpCode::Constants::OP_BRSHIFT,  Definition{ "OP_BRSHIFT", {} } },
	{ OpCode::Constants::OP_EQUAL,    Definition{ "OP_EQUAL", {} } },
	{ OpCode::Constants::OP_NOT_EQUAL,Definition{ "OP_NOT_EQUAL", {} } },
	{ OpCode::Constants::OP_GREATER_THAN, Definition{ "OP_GREATER_THAN", {} } },
	{ OpCode::Constants::OP_GREATER_THAN_EQUAL, Definition{ "OP_GREATER_THAN_EQUAL", {} } },
	{ OpCode::Constants::OP_LESS_THAN, Definition{ "OP_LESS_THAN", {} } },
	{ OpCode::Constants::OP_LESS_THAN_EQUAL, Definition{ "OP_LESS_THAN_EQUAL", {} } },
	{ OpCode::Constants::OP_BOOL_AND, Definition{ "OP_BOOL_AND", {} } },
	{ OpCode::Constants::OP_BOOL_OR, Definition{ "OP_BOOL_OR", {} } },
	{ OpCode::Constants::OP_NEGATE, Definition{ "OP_NEGATE", {} } },
	{ OpCode::Constants::OP_NOT, Definition{ "OP_NOT", {} } },
	{ OpCode::Constants::OP_BNOT, Definition{ "OP_BNOT", {} } },
};

std::variant<Definition, std::string> OpCode::Lookup(const OpCode::Constants opcode)
{
	auto it = Definitions.find(opcode);
	if (it != Definitions.end())
	{
		return it->second;
	}
	return std::format("Opcode {} not found", static_cast<Opcode>(opcode));
}

Instructions OpCode::Make(OpCode::Constants opcode, std::vector<int> operands)
{
	auto def = Lookup(opcode);
	if (std::holds_alternative<std::string>(def))
	{
		throw std::runtime_error(std::get<std::string>(def));
	}
	auto definition = std::get<Definition>(def);
	Instructions instructions;
	instructions.push_back(static_cast<uint8_t>(opcode));
	for (size_t i = 0; i < definition.OperandWidths.size(); i++)
	{
		auto operand = operands[i];
		auto width = definition.OperandWidths[i];
		switch (width)
		{
		case 2:
			instructions.push_back((operand >> 8) & 0xFF);
			instructions.push_back(operand & 0xFF);
			break;
		default:
			throw std::runtime_error("Invalid operand width");
		}
	}
	return instructions;
}

std::tuple<OpCode::Constants, std::vector<int>, size_t> OpCode::ReadOperand(const Instructions& instructions, size_t offset)
{
	if (instructions.size() < 2)
	{
		throw std::runtime_error("No instructions");
	}

	auto read = offset;
	auto opcode = static_cast<OpCode::Constants>(instructions[offset]);
	auto def = Lookup(opcode);
	if (std::holds_alternative<std::string>(def))
	{
		throw std::runtime_error(std::get<std::string>(def));
	}
	read += sizeof(Opcode);

	auto definition = std::get<Definition>(def);

	std::vector<int> operands;
	for (size_t i = 0; i < definition.OperandWidths.size(); i++)
	{
		if (instructions.size() < read + definition.OperandWidths[i])
		{
			throw std::runtime_error("No operand where one expected");
		}

		auto width = definition.OperandWidths[i];
		switch (width)
		{
		case 2:
			operands.push_back((instructions[read] << 8) | instructions[read + 1]);
			break;
		default:
			throw std::runtime_error("Invalid operand width");
		}
		read += width;
	}
	return { opcode, operands, read };
}

OpCode::Constants OpCode::GetOpcode(const Instructions& instructions, size_t offset)
{
	if (instructions.size() < 2)
	{
		throw std::runtime_error("No instructions");
	}

	auto read = offset;
	auto opcode = static_cast<OpCode::Constants>(instructions[offset]);
	return opcode;
}

std::string OpCode::PrintInstructions(const Instructions& instructions)
{
	std::string result;
	size_t offset = 0;

	while(offset < instructions.size())
	{
		auto [op, operands, readOffset] = OpCode::ReadOperand(instructions, offset);
		result += std::format("{:0>4}:  {:16}", offset, std::get<Definition>(OpCode::Lookup(op)).Name);
		for (const auto& operand : operands)
		{
			result += std::format("{:8}", operand);
		}
		result += "\n";
		offset = readOffset;
	}
	return result;
}