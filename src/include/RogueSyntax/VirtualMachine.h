#pragma once
#include <StandardLib.h>
#include <OpCode.h>


#define STACK_SIZE 2048

class RogueVM
{
public:
	RogueVM(const ByteCode& byteCode);
	~RogueVM();

	void Run();

	//stack operations
	std::shared_ptr<IObject> Top() const;
	void Push(std::shared_ptr<IObject> obj);
	std::shared_ptr<IObject> Pop();
	std::shared_ptr<IObject> LastPoppped() const;

	static std::shared_ptr<RogueVM> New(const ByteCode& byteCode) { return std::make_shared<RogueVM>(byteCode); }

protected:
	void ExecuteArithmeticInfix(OpCode::Constants opcode);
	void ExecuteIntegerArithmeticInfix(OpCode::Constants opcode, IntegerObj left, IntegerObj right);
	void ExecuteDecimalArithmeticInfix(OpCode::Constants opcode, DecimalObj left, DecimalObj right);
	void ExecuteStringArithmeticInfix(OpCode::Constants opcode, StringObj left, StringObj right);
	
	void ExecuteComparisonInfix(OpCode::Constants opcode);
	void ExecuteIntegerComparisonInfix(OpCode::Constants opcode, IntegerObj left, IntegerObj right);
	void ExecuteDecimalComparisonInfix(OpCode::Constants opcode, DecimalObj left, DecimalObj right);
	void ExecuteStringComparisonInfix(OpCode::Constants opcode, StringObj left, StringObj right);
	void ExecuteBooleanComparisonInfix(OpCode::Constants opcode, BooleanObj left, BooleanObj right);
	void ExecuteNullComparisonInfix(OpCode::Constants opcode, NullObj left, NullObj right);


	std::string MakeOpCodeError(const std::string& message, OpCode::Constants opcode);

private:
	ByteCode _byteCode;
	std::array<std::shared_ptr<IObject>, STACK_SIZE> _stack;
	uint16_t _sp = 0;
	
};