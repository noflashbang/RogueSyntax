#include "pch.h"

RogueVM::RogueVM(const ByteCode& byteCode)
	: _byteCode(byteCode)
{
}

RogueVM::~RogueVM()
{
}

void RogueVM::Run()
{
	auto instructions = _byteCode.Instructions;
	auto constants = _byteCode.Constants;
	size_t ip = 0;
	while (ip < instructions.size())
	{
		auto opcode = OpCode::GetOpcode(instructions, ip);
		ip++;
			
		switch (opcode)
		{
		case OpCode::Constants::OP_CONSTANT:
		{
			auto idx = instructions[ip] << 8 | instructions[ip + 1];
			auto constant = constants[idx];
			//std::cout << constant->Inspect() << std::endl;
			ip += 2;

			Push(constant);
			break;
		}
		case OpCode::Constants::OP_ADD:
		{
			auto right = Pop();
			auto left = Pop();
			if (left->Type() != right->Type())
			{
				throw std::runtime_error("Type mismatch");
			}
			if (left->Type() == ObjectType::INTEGER_OBJ)
			{
				auto result = IntegerObj::New(std::dynamic_pointer_cast<IntegerObj>(left)->Value + std::dynamic_pointer_cast<IntegerObj>(right)->Value);
				Push(result);
			}
			else if (left->Type() == ObjectType::DECIMAL_OBJ)
			{
				auto result = DecimalObj::New(std::dynamic_pointer_cast<DecimalObj>(left)->Value + std::dynamic_pointer_cast<DecimalObj>(right)->Value);
				Push(result);
			}
			else
			{
				throw std::runtime_error("Unsupported type");
			}
			break;
		}
		default:
			throw std::runtime_error("Unknown opcode");
		}
	}
}