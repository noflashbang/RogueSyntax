#include "OpCode.h"
#include "OpCode.h"
#include <pch.h>

const std::unordered_map<OpCode::Constants, Definition> OpCode::Definitions = {
	{ OpCode::Constants::OP_CONSTANT,    Definition{ "OP_CONSTANT", { 2 } } },
	{ OpCode::Constants::OP_LINT,        Definition{ "OP_LINT", { 4 } } },
	{ OpCode::Constants::OP_LDECIMAL,    Definition{ "OP_LDECIMAL", { 4 } } },
	{ OpCode::Constants::OP_LSTRING,     Definition{ "OP_LSTRING", { 4 } } },
	{ OpCode::Constants::OP_LFUN,        Definition{ "OP_LFUN", { 2, 2, 4 } } },
	{ OpCode::Constants::OP_FALSE,       Definition{ "OP_FALSE", {} } },
	{ OpCode::Constants::OP_TRUE,        Definition{ "OP_TRUE", {} } },
	{ OpCode::Constants::OP_NULL,        Definition{ "OP_NULL", {} } },
	{ OpCode::Constants::OP_ARRAY,       Definition{ "OP_ARRAY", {2} } },
	{ OpCode::Constants::OP_HASH,        Definition{ "OP_HASH", {2} } },
	{ OpCode::Constants::OP_POP,         Definition{ "OP_POP", {} } },
	{ OpCode::Constants::OP_ADD,         Definition{ "OP_ADD", {} } },
	{ OpCode::Constants::OP_SUB,         Definition{ "OP_SUB", {} } },
	{ OpCode::Constants::OP_MUL,         Definition{ "OP_MUL", {} } },
	{ OpCode::Constants::OP_DIV,         Definition{ "OP_DIV", {} } },
	{ OpCode::Constants::OP_MOD,         Definition{ "OP_MOD", {} } },
	{ OpCode::Constants::OP_BOR,         Definition{ "OP_BOR", {} } },
	{ OpCode::Constants::OP_BAND,        Definition{ "OP_BAND", {} } },
	{ OpCode::Constants::OP_BXOR,        Definition{ "OP_BXOR", {} } },
	{ OpCode::Constants::OP_BLSHIFT,     Definition{ "OP_BLSHIFT", {} } },
	{ OpCode::Constants::OP_BRSHIFT,     Definition{ "OP_BRSHIFT", {} } },
	{ OpCode::Constants::OP_EQ,          Definition{ "OP_EQUAL", {} } },
	{ OpCode::Constants::OP_NEQ,         Definition{ "OP_NOT_EQUAL", {} } },
	{ OpCode::Constants::OP_GT,          Definition{ "OP_GT", {} } },
	{ OpCode::Constants::OP_GTE,         Definition{ "OP_GTE", {} } },
	{ OpCode::Constants::OP_LT,          Definition{ "OP_LT", {} } },
	{ OpCode::Constants::OP_LTE,	     Definition{ "OP_LTE", {} } },
	{ OpCode::Constants::OP_AND,	     Definition{ "OP_AND", {} } },
	{ OpCode::Constants::OP_OR,          Definition{ "OP_OR", {} } },
	{ OpCode::Constants::OP_NEGATE,      Definition{ "OP_NEGATE", {} } },
	{ OpCode::Constants::OP_NOT,         Definition{ "OP_NOT", {} } },
	{ OpCode::Constants::OP_BNOT,        Definition{ "OP_BNOT", {} } },
	{ OpCode::Constants::OP_JUMP,        Definition{ "OP_JUMP", { 2 } } },
	{ OpCode::Constants::OP_JUMPIFZ,     Definition{ "OP_JUMPIFZ", { 2 } } },
	{ OpCode::Constants::OP_GET,         Definition{ "OP_GET", { 2 } } },
	{ OpCode::Constants::OP_SET,         Definition{ "OP_SET", { 2 } } },
	{ OpCode::Constants::OP_SET_ASSIGN,  Definition{ "OP_SET_ASSIGN", {2} } },
	{ OpCode::Constants::OP_INDEX,       Definition{ "OP_INDEX", {} } },
	{ OpCode::Constants::OP_CALL,        Definition{ "OP_CALL", { 2 } } },
	{ OpCode::Constants::OP_CLOSURE,     Definition{ "OP_CLOSURE", { 2 } } },
	{ OpCode::Constants::OP_RETURN,      Definition{ "OP_RETURN", {} } },
	{ OpCode::Constants::OP_RET_VAL,     Definition{ "OP_RET_VAL", {} } },
	{ OpCode::Constants::OP_CUR_CLOSURE, Definition{ "OP_CUR_CLOSURE", {} } },
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

Instructions OpCode::Make(OpCode::Constants opcode, std::vector<uint32_t> operands)
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
		case 4:
			instructions.push_back((operand >> 24) & 0xFF);
			instructions.push_back((operand >> 16) & 0xFF);
			instructions.push_back((operand >> 8) & 0xFF);
			instructions.push_back(operand & 0xFF);
			break;
		default:
			throw std::runtime_error("Invalid operand width");
		}
	}
	return instructions;
}

Instructions OpCode::Make(OpCode::Constants opcode, std::vector<uint32_t> operands, Instructions data)
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
		case 4:
			instructions.push_back((operand >> 24) & 0xFF);
			instructions.push_back((operand >> 16) & 0xFF);
			instructions.push_back((operand >> 8) & 0xFF);
			instructions.push_back(operand & 0xFF);
			break;
		default:
			throw std::runtime_error("Invalid operand width");
		}
	}
	instructions.insert(instructions.end(), data.begin(), data.end());
	return instructions;
}

Instructions OpCode::MakeIntegerLiteral(int value)
{
	auto val = reinterpret_cast<uint32_t&>(value);
	return Make(OpCode::Constants::OP_LINT, { val });
}

Instructions OpCode::MakeDecimalLiteral(float value)
{
	auto val = reinterpret_cast<uint32_t&>(value);
	return Make(OpCode::Constants::OP_LDECIMAL, { val });
}

Instructions OpCode::MakeStringLiteral(const std::string& value)
{
	unsigned len = value.size();
	Instructions data;
	for (auto c : value)
	{
		data.push_back(c);
	}
	return Make(OpCode::Constants::OP_LSTRING, { len }, data);
}

std::tuple<OpCode::Constants, std::vector<uint32_t>, size_t> OpCode::ReadOperand(const Instructions& instructions, size_t offset)
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

	std::vector<uint32_t> operands;
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
		case 4:
			operands.push_back((instructions[read] << 24) | (instructions[read + 1] << 16) | (instructions[read + 2] << 8) | instructions[read + 3]);
			break;
		default:
			throw std::runtime_error("Invalid operand width");
		}
		read += width;
	}
	return { opcode, operands, read };
}

bool OpCode::HasData(OpCode::Constants opcode)
{
	return opcode == OpCode::Constants::OP_LSTRING;
}

std::vector<uint8_t> OpCode::ReadData(std::tuple<OpCode::Constants, std::vector<uint32_t>, size_t> data, const Instructions& instructions)
{
	auto [opcode, operands, readOffset] = data;
	std::vector<uint8_t> result;
	if (opcode == OpCode::Constants::OP_LSTRING)
	{
		for (size_t i = readOffset; i < readOffset + operands[0]; i++)
		{
			result.push_back(instructions[i]);
		}
	}
	else if (opcode == OpCode::Constants::OP_LFUN)
	{
		auto size = operands[2];
		for (size_t i = readOffset; i < readOffset + size; i++)
		{
			result.push_back(instructions[i]);
		}
	}
	return result;
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
	std::string result{};
	size_t offset{};
	std::span instructionsSpan(instructions);


	while(offset < instructions.size())
	{
		std::string decompiled;
		std::string rawByteCode;
		auto [op, operands, readOffset] = OpCode::ReadOperand(instructions, offset);
		decompiled += std::format("{:0>4}:  {:16}", offset, std::get<Definition>(OpCode::Lookup(op)).Name);
		if (operands.size() == 1)
		{
			decompiled += std::format("{: <18}", operands[0]);
		}
		else if (operands.size() == 2)
		{
			decompiled += std::format("{: <6}", operands[0]);
			decompiled += std::format("{: <6}", operands[1]);
			decompiled += "      ";

		}
		else if (operands.size() == 3)
		{
			decompiled += std::format("{: <6}", operands[0]);
			decompiled += std::format("{: <6}", operands[1]);
			decompiled += std::format("{: <6}", operands[2]);
		}
		else
		{
			for (const auto& operand : operands)
			{
				decompiled += std::format("{: <6}", operand);
			}
		}

		if (op == OpCode::Constants::OP_GET || op == OpCode::Constants::OP_SET)
		{
			auto adjustedIdx = AdjustIdx(operands[0]);
			auto type = GetTypeFromIdx(operands[0]);
			switch (type)
			{
			case ScopeType::SCOPE_GLOBAL:
				decompiled += std::format(" // {: <6} {: <2}", "GLOBAL", adjustedIdx);
				break;
			case ScopeType::SCOPE_LOCAL:
				decompiled += std::format(" // {: <6} {: <2}", "LOCAL", adjustedIdx);
				break;
			case ScopeType::SCOPE_EXTERN:
				decompiled += std::format(" // {: <6} {: <2}", "EXTERN", adjustedIdx);
				break;
			case ScopeType::SCOPE_FREE:
				decompiled += std::format(" // {: <6} {: <2}", "FREE", adjustedIdx);
				break;
			}
		}

		if (HasData(op))
		{
			auto data = OpCode::ReadData({ op, operands, readOffset }, instructions);
			if (op == OpCode::Constants::OP_LSTRING)
			{
				std::string str(data.begin(), data.end());
				decompiled += std::format(" // '{}'", str);
				readOffset += data.size();
			}
		}

		auto raw = instructionsSpan.subspan(offset, readOffset - offset);
		rawByteCode += InstructionsToHex(raw);
		result += rawByteCode + " : " + decompiled + "\n";
		offset = readOffset;
	}
	result.pop_back();
	return result;
}

std::string OpCode::PrintInstructionsWithDebug(const ByteCode& code)
{
	std::vector<std::string> decompiledList;
	std::vector<std::string> debugSymbolsList;

	std::string result{};
	size_t offset{};
	std::span instructionsSpan(code.Instructions);

	auto maxDebugLine = std::ranges::max_element(code.DebugSymbols, [](const auto& lhs, const auto& rhs) { return lhs.SourceAst.size() < rhs.SourceAst.size(); }); 
	auto maxDebugLineSize = maxDebugLine->SourceAst.size() + 6; // -> /* {} */

	while (offset < code.Instructions.size())
	{
		std::string decompiled;
		std::string rawByteCode;
		auto [op, operands, readOffset] = OpCode::ReadOperand(code.Instructions, offset);
		decompiled += std::format("{:0>4}:  {:16}", offset, std::get<Definition>(OpCode::Lookup(op)).Name);
		if (operands.size() == 1)
		{
			decompiled += std::format("{: <18}", operands[0]);
		}
		else if (operands.size() == 2)
		{
			decompiled += std::format("{: <6}", operands[0]);
			decompiled += std::format("{: <6}", operands[1]);
			decompiled += "      ";

		}
		else if (operands.size() == 3)
		{
			decompiled += std::format("{: <6}", operands[0]);
			decompiled += std::format("{: <6}", operands[1]);
			decompiled += std::format("{: <6}", operands[2]);
		}
		else
		{
			for (const auto& operand : operands)
			{
				decompiled += std::format("{: <6}", operand);
			}
		}

		if (op == OpCode::Constants::OP_GET || op == OpCode::Constants::OP_SET)
		{
			auto adjustedIdx = AdjustIdx(operands[0]);
			auto type = GetTypeFromIdx(operands[0]);
			switch (type)
			{
			case ScopeType::SCOPE_GLOBAL:
				decompiled += std::format(" // {: <6} {: <2}", "GLOBAL", adjustedIdx);
				break;
			case ScopeType::SCOPE_LOCAL:
				decompiled += std::format(" // {: <6} {: <2}", "LOCAL", adjustedIdx);
				break;
			case ScopeType::SCOPE_EXTERN:
				decompiled += std::format(" // {: <6} {: <2}", "EXTERN", adjustedIdx);
				break;
			case ScopeType::SCOPE_FREE:
				decompiled += std::format(" // {: <6} {: <2}", "FREE", adjustedIdx);
				break;
			}
		}

		if (HasData(op))
		{
			auto data = OpCode::ReadData({ op, operands, readOffset }, code.Instructions);
			if (op == OpCode::Constants::OP_LSTRING)
			{
				std::string str(data.begin(), data.end());
				decompiled += std::format(" // '{}'", str);
				readOffset += data.size();
			}
		}

		std::string debugInfo = "";
		auto debug = std::find_if(code.DebugSymbols.begin(), code.DebugSymbols.end(), [offset](const auto& symbol) { return symbol.Offset == offset; });
		if (debug != code.DebugSymbols.end())
		{
			debugInfo += std::format("/* {: <{}} */ LN:{} CH:{}", debug->SourceAst, maxDebugLineSize, debug->BaseToken.Location.Line, debug->BaseToken.Location.Character);
		}
		else
		{
			debugInfo += std::format("/* {: <{}} */", "", maxDebugLineSize);
		}

		debugSymbolsList.push_back(debugInfo);

		auto raw = instructionsSpan.subspan(offset, readOffset - offset);
		rawByteCode += InstructionsToHex(raw);

		auto bytes = InstructionsToHexVec(raw, 5);
		if (bytes.size() > 1)
		{
			for (auto iter = 0; iter < bytes.size() - 1; iter++)
			{
				decompiledList.push_back(bytes[iter]);
				debugSymbolsList.push_back("");
			}
		}
		if (bytes.size() > 0)
		{
			decompiledList.push_back(bytes[bytes.size() - 1] + " : " + decompiled);
		}
		
		offset = readOffset;
	}

	auto maxLength = std::ranges::max_element(decompiledList, [](const auto& lhs, const auto& rhs) { return lhs.size() < rhs.size(); })->size();

	for (size_t i = 0; i < decompiledList.size(); i++)
	{
		result += decompiledList[i] + std::string(maxLength - decompiledList[i].size(), ' ') + debugSymbolsList[i] + "\n";
	}
	result.pop_back();
	return result;
}

std::string OpCode::PrintInstuctionsCompared(const Instructions& instructions, const Instructions& otherInstructions)
{
	using namespace std::literals;
	std::string result;
	
	auto left = OpCode::PrintInstructions(instructions);
	auto right = OpCode::PrintInstructions(otherInstructions);
	constexpr auto delimiter{ "\n"sv  };
	auto leftLines = std::views::split(left, delimiter);
	auto rightLines = std::views::split(right, delimiter);

	auto lengths = std::views::transform([](const auto&& line) { return std::ranges::distance(line); });
	auto maxLengthLeft = std::ranges::max(lengths(leftLines));
	auto maxLengthRight = std::ranges::max(lengths(rightLines));
	auto maxLength = std::max(maxLengthLeft, maxLengthRight);

	auto leftIt = leftLines.begin();
	auto rightIt = rightLines.begin();

	while (leftIt != leftLines.end() || rightIt != rightLines.end())
	{
		if (leftIt != leftLines.end())
		{
			std::string leftLine((*leftIt).begin(), (*leftIt).end());
			result += std::format("{: <{}}", leftLine, maxLength);
			++leftIt;
		}
		else
		{
			result += std::format("{:->{}}", "", maxLength);
		}

		result += " | ";

		if (rightIt != rightLines.end())
		{
			std::string rightLine((*rightIt).begin(), (*rightIt).end());
			result += std::format("{: <{}}", rightLine, maxLength);
			++rightIt;
		}
		else
		{
			result += std::format("{:->{}}", "", maxLength);
		}
		result += "\n";
	}
	return result;
}

std::string OpCode::InstructionsToHex(std::span<const uint8_t> bytes)
{
	auto vec = InstructionsToHexVec(bytes, 5);
	std::string ret;

	for (auto& str : vec)
	{
		ret += str + '\n';
	}

	ret.pop_back();
	return ret;
}

std::vector<std::string> OpCode::InstructionsToHexVec(std::span<const uint8_t> bytes, size_t width)
{
	std::vector<std::string> ret;
	auto cnt = 0;
	std::ostringstream oss;
	for (const auto& byte : bytes) {
		oss << std::format("{:02X} ", byte);
		cnt++;
		if (cnt % width == 0 && cnt != bytes.size())
		{
			ret.push_back(oss.str());
			oss.str("");
			oss.clear();
		}
	}

	if (cnt % width != 0)
	{
		oss << std::string((width - (cnt % width)) * 3, ' ');
		
	}
	ret.push_back(oss.str());
	return ret;
}