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

void CompilationUnit::AddDebugSymbol(const Token& baseToken, const Symbol* sym, const std::string& astStr)
{
	size_t index = 0;
	std::string symbol = "";
	std::string scope = "";
	if (sym != nullptr)
	{
		index = sym->Index;
		symbol = sym->MangledName;
		if (sym->Type == ScopeType::SCOPE_FUNCTION)
		{
			scope = "FUNCTION";
		}
		else if(sym->Type == ScopeType::SCOPE_EXTERN)
		{
			scope = "EXTERN";
		}
		else if (sym->Type == ScopeType::SCOPE_FREE)
		{
			scope = "FREE";
		}
		else if (sym->Type == ScopeType::SCOPE_GLOBAL)
		{
			scope = "GLOBAL";
		}
		else if (sym->Type == ScopeType::SCOPE_LOCAL)
		{
			scope = "LOCAL";
		}
	}

	DebugSymbols.push_back(DebugSymbol{ UnitInstructions.size(), baseToken, index, symbol, scope, astStr });
}

void CompilationUnit::AddDebugSymbol(size_t offset, const DebugSymbol& ds)
{
	DebugSymbols.push_back(DebugSymbol{ offset, ds.BaseToken, ds.Index, ds.Symbol, ds.Scope, ds.SourceAst });
}

void CompilationUnit::SetLastInstruction(const Instructions& instruction)
{
	PreviousLastInstruction = LastInstruction;
	LastInstruction = instruction;
}
