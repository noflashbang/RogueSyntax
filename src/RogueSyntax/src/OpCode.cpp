#include <pch.h>

const std::unordered_map<OpCodeConstants, Definition> StdOpCode::OpCodeDefinitions = {
	{ OpCodeConstants::OP_CONSTANT, Definition{ "OP_CONSTANT", { 2 } } },
};

std::variant<Definition, std::string> StdOpCode::Lookup(const OpCodeConstants opcode)
{
	auto it = OpCodeDefinitions.find(opcode);
	if (it != OpCodeDefinitions.end())
	{
		return it->second;
	}
	return std::format("Opcode {} not found", static_cast<Opcode>(opcode));
}

Instructions StdOpCode::Make(OpCodeConstants opcode, std::vector<int> operands)
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

std::tuple<OpCodeConstants, std::vector<int>, size_t> StdOpCode::ReadOperand(const Instructions& instructions, size_t offset)
{
	if (instructions.size() < 2)
	{
		throw std::runtime_error("No instructions");
	}

	if (instructions.size() < offset + 2)
	{
		throw std::runtime_error("No operand");
	}

	auto read = offset;
	auto opcode = static_cast<OpCodeConstants>(instructions[offset]);
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

std::string StdOpCode::PrintInstructions(const Instructions& instructions)
{
	std::string result;
	size_t offset = 0;

	while(offset < instructions.size())
	{
		auto [op, operands, readOffset] = StdOpCode::ReadOperand(instructions, offset);
		result += std::format("{:0>4}:  {:16}", offset, std::get<Definition>(StdOpCode::Lookup(op)).Name);
		for (const auto& operand : operands)
		{
			result += std::format("{:8}", operand);
		}
		result += "\n";
		offset = readOffset;
	}
	return result;
}