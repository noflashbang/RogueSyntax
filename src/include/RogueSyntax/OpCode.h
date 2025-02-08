#pragma once
#include <StandardLib.h>
#include <variant>

#include <Token.h>

typedef std::vector<uint8_t> Instructions;

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
		throw std::runtime_error("Invalid index");
	}
}

static inline int AdjustIdx(int idx)
{
	return idx & 0x3FFF;
}

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
	Token BaseToken;
	std::string SourceAst;
};

struct ObjectCode
{
	Instructions Instructions;
	std::vector<Symbol> Symbols;
	std::vector<DebugSymbol> DebugSymbols;
};

struct ByteCode
{
	Instructions Instructions;
	std::vector<DebugSymbol> DebugSymbols;
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
	static Instructions Make(Constants opcode, std::vector<uint32_t> operands);
	static Instructions Make(Constants opcode, std::vector<uint32_t> operands, Instructions data);

	static Instructions MakeIntegerLiteral(int value);
	static Instructions MakeDecimalLiteral(float value);
	static Instructions MakeStringLiteral(const std::string& value);

	static std::tuple<Constants, std::vector<uint32_t>, size_t> ReadOperand(const Instructions& instructions, size_t offset);
	static bool HasData(Constants opcode);
	static std::vector<uint8_t> ReadData(std::tuple<Constants, std::vector<uint32_t>, size_t>, const Instructions& instructions);
	static Constants GetOpcode(const Instructions& instructions, size_t offset);
	static std::string PrintInstructions(const Instructions& instructions);
	static std::string PrintInstructionsWithDebug(const ByteCode& code);

	static std::string PrintInstuctionsCompared(const Instructions& instructions, const Instructions& otherInstructions);
	static std::string InstructionsToHex(std::span<const uint8_t> bytes);
	
};


