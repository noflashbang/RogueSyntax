#include "CompilationUnit.h"
#include <pch.h>


int CompilationUnit::AddInstruction(RSInstructions instructions)
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

void CompilationUnit::ReplaceInstruction(int position, RSInstructions instructions)
{
	for (const auto& instr : instructions)
	{
		UnitInstructions[position++] = instr;
	}
}

void CompilationUnit::SetLastInstruction(const RSInstructions& instruction)
{
	PreviousLastInstruction = LastInstruction;
	LastInstruction = instruction;
}
