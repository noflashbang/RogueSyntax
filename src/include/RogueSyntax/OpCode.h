#pragma once
#include <StandardLib.h>
#include <variant>

#include <Token.h>
typedef std::vector<uint8_t> RSInstructions;



struct Definition
{
	std::string Name;
	std::vector<uint16_t> OperandWidths;
};

class IObject;

enum class ScopeType : uint8_t
{
	SCOPE_GLOBAL,
	SCOPE_LOCAL,
	SCOPE_EXTERN,
	SCOPE_FREE,
	SCOPE_FUNCTION,
};

static inline ScopeType GetTypeFromIdx(int idx)
{
	auto flags = idx & 0xC000;
	if (flags == 0x0000)
	{
		return ScopeType::SCOPE_GLOBAL;
	}
	else if (flags == 0x8000)
	{
		return ScopeType::SCOPE_LOCAL;
	}
	else if (flags == 0x4000)
	{
		return ScopeType::SCOPE_EXTERN;
	}
	else if (flags == 0xC000)
	{
		return ScopeType::SCOPE_FREE;
	}
	else
	{
		throw std::runtime_error(std::format("Invalid index {}", idx));
	}
};

static inline int AdjustIdx(int idx)
{
	return idx & 0x3FFF;
};

struct Symbol
{
	ScopeType Type;
	std::string Name;
	std::string MangledName;
	std::string Context;
	uint32_t stackContext;
	int Index;

	uint32_t EncodedIdx();
};

struct DebugSymbol
{
	size_t Offset;
	RSToken BaseToken;
	size_t Index;
	std::string Symbol;
	std::string Scope;
	std::string SourceAst;
};

struct ObjectCode
{
	RSInstructions Instructions;
	std::vector<Symbol> Symbols;
	std::vector<DebugSymbol> DebugSymbols;
};

struct ByteCode
{
	RSInstructions Instructions;
	std::vector<DebugSymbol> DebugSymbols;
};

struct DissaemblyDetail
{
	size_t InstructionOffset;
	std::string ByteCode;
	std::string Instruction;
	std::array<uint16_t, 4> Operands;
	std::vector<uint8_t> Data;
	std::string ScopeType;
	size_t Index;
};

struct OpCode
{
	typedef uint8_t Opcode;

	enum class Constants : Opcode
	{
		//literals
		OP_CONSTANT,       //this is really a placeholder for the actual type
		OP_LINT,      //integer literal
		OP_LDECIMAL,  //decimal literal
		OP_LSTRING,   //string literal
		OP_LFUN, //function obj
		//types
		OP_TRUE,
		OP_FALSE,
		OP_NULL,
		OP_ARRAY,
		OP_HASH,
		//stack
		OP_POP,
		//arithmetic
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
		//comparison
		OP_EQ,
		OP_NEQ,
		OP_GT,
		OP_GTE,
		OP_LT,
		OP_LTE,
		OP_AND,
		OP_OR,
		//prefix
		OP_NEGATE,
		OP_NOT,
		OP_BNOT,
		//jump
		OP_JUMP,
		OP_JUMPIFZ,
		//mem
		OP_GET,
		OP_SET,
		//OP_GET_GLOBAL,
		//OP_SET_GLOBAL,
		//OP_GET_EXTRN,
		//OP_GET_FREE,
		OP_SET_ASSIGN,
		//op
		OP_INDEX,
		OP_CALL,
		OP_CLOSURE,
		OP_RETURN,
		OP_RET_VAL,
		OP_CUR_CLOSURE,
	};

	static const std::unordered_map<Constants, Definition> Definitions;
	static std::variant<Definition, std::string> Lookup(const Constants opcode);
	static RSInstructions Make(Constants opcode, std::vector<uint32_t> operands);
	static RSInstructions Make(Constants opcode, std::vector<uint32_t> operands, RSInstructions data);

	static RSInstructions MakeIntegerLiteral(int value);
	static RSInstructions MakeDecimalLiteral(float value);
	static RSInstructions MakeStringLiteral(const std::string& value);

	static std::tuple<Constants, std::vector<uint32_t>, size_t> ReadOperand(const RSInstructions& instructions, size_t offset);
	static bool HasData(Constants opcode);
	static std::vector<uint8_t> ReadData(std::tuple<Constants, std::vector<uint32_t>, size_t>, const RSInstructions& instructions);
	static Constants GetOpcode(const RSInstructions& instructions, size_t offset);
	static std::string PrintInstructions(const RSInstructions& instructions);
	static std::string PrintInstructionsWithDebug(const ByteCode& code);

	static std::vector<DissaemblyDetail> Disassemble(const RSInstructions& instructions);
	static std::string PrintDisassemblyDetail(const DissaemblyDetail& detail);

	static std::string PrintInstuctionsCompared(const RSInstructions& instructions, const RSInstructions& otherInstructions);
	static std::string InstructionsToHex(std::span<const uint8_t> bytes);
	static std::vector<std::string> InstructionsToHexVec(std::span<const uint8_t> bytes, size_t width);
	
};


