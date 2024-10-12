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

	Instructions UnitInstructions;
	Instructions LastInstruction;
	Instructions PreviousLastInstruction;

	std::stack<LoopJump> LoopJumps;

	void SetLastInstruction(const Instructions& instruction);
	int AddInstruction(Instructions instructions);

	void RemoveLastPop();
	bool LastInstructionIs(OpCode::Constants opcode);
	void RemoveLastInstruction();
	void ChangeOperand(int position, uint32_t operand);
	void ReplaceInstruction(int position, Instructions instructions);
};

