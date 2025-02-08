#include "CompilationUnit.h"
#include <pch.h>


int CompilationUnit::AddInstruction(Instructions instructions)
{
	auto position = UnitInstructions.size();
	UnitInstructions.insert(UnitInstructions.end(), instructions.begin(), instructions.end());

	SetLastInstruction(instructions);
	return position;
}

void CompilationUnit::RemoveLastPop()
{
	if (LastInstructionIs(OpCode::Constants::OP_POP))
	{
		RemoveLastInstruction();
	}
}

bool CompilationUnit::LastInstructionIs(OpCode::Constants opcode)
{
	if (LastInstruction.empty())
	{
		return false;
	}

	return LastInstruction[0] == static_cast<uint8_t>(opcode);
}

void CompilationUnit::RemoveLastInstruction()
{
	UnitInstructions.resize(UnitInstructions.size() - LastInstruction.size());
	SetLastInstruction(PreviousLastInstruction);
}

void CompilationUnit::ChangeOperand(int position, uint32_t operand)
{
	auto instruction = OpCode::Make(static_cast<OpCode::Constants>(UnitInstructions[position]), { operand });
	ReplaceInstruction(position, instruction);
}

void CompilationUnit::ReplaceInstruction(int position, Instructions instructions)
{
	for (const auto& instr : instructions)
	{
		UnitInstructions[position++] = instr;
	}
}

void CompilationUnit::AddDebugSymbol(const Token& baseToken, const std::string& astStr)
{
	DebugSymbols.push_back(DebugSymbol{ UnitInstructions.size(), baseToken, astStr });
}

void CompilationUnit::AddDebugSymbol(size_t offest, const Token& baseToken, const std::string& astStr)
{
	DebugSymbols.push_back(DebugSymbol{ offest, baseToken, astStr });
}

void CompilationUnit::SetLastInstruction(const Instructions& instruction)
{
	PreviousLastInstruction = LastInstruction;
	LastInstruction = instruction;
}
