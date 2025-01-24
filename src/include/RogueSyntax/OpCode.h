#pragma once
#include <StandardLib.h>
#include <variant>

typedef std::vector<uint8_t> Instructions;

struct Definition
{
	std::string Name;
	std::vector<uint16_t> OperandWidths;
};

struct OpCode
{
	typedef uint8_t Opcode;

	enum class Constants : Opcode
	{
		//literals
		OP_CONSTANT,       //this is really a placeholder for the actual type
		OP_CONST_INT,      //integer literal
		OP_CONST_DECIMAL,  //decimal literal
		OP_CONST_STRING,   //string literal
		OP_CONST_FUNCTION, //function obj
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
		OP_EQUAL,
		OP_NOT_EQUAL,
		OP_GREATER_THAN,
		OP_GREATER_THAN_EQUAL,
		OP_LESS_THAN,
		OP_LESS_THAN_EQUAL,
		OP_BOOL_AND,
		OP_BOOL_OR,
		//prefix
		OP_NEGATE,
		OP_NOT,
		OP_BNOT,
		//jump
		OP_JUMP,
		OP_JUMP_IF_FALSE,
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
		OP_RETURN_VALUE,
		OP_CURRENT_CLOSURE,
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

	static std::string PrintInstuctionsCompared(const Instructions& instructions, const Instructions& otherInstructions);
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

struct ObjectCode
{
	Instructions Instructions;
	std::vector<const IObject*> Constants;
	std::vector<Symbol> Symbols;
};

struct ByteCode
{
	Instructions Instructions;
	std::vector<const IObject*> Constants;
};
