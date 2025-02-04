#pragma once
#include <StandardLib.h>

enum LoopJumpType
{
	LOOP_JUMP_CONTINUE,
	LOOP_JUMP_BREAK,
};

struct LoopJump
{
	LoopJumpType Type;
	int Instruction;
};

struct CompilationUnit
{
	CompilationUnit()
	{
	}

	RSInstructions UnitInstructions;
	RSInstructions LastInstruction;
	RSInstructions PreviousLastInstruction;

	std::stack<LoopJump> LoopJumps;

	void SetLastInstruction(const RSInstructions& instruction);
	int AddInstruction(RSInstructions instructions);

	void RemoveLastPop();
	bool LastInstructionIs(OpCode::Constants opcode);
	void RemoveLastInstruction();
	void ChangeOperand(int position, uint32_t operand);
	void ReplaceInstruction(int position, RSInstructions instructions);
};

