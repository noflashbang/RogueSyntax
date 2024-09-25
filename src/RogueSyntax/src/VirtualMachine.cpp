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
		case OpCode::Constants::OP_SUB:
		case OpCode::Constants::OP_MUL:
		case OpCode::Constants::OP_DIV:
		case OpCode::Constants::OP_MOD:
		case OpCode::Constants::OP_BOR:
		case OpCode::Constants::OP_BAND:
		case OpCode::Constants::OP_BXOR:
		case OpCode::Constants::OP_BLSHIFT:
		case OpCode::Constants::OP_BRSHIFT:
		{
			ExecuteArithmeticInfix(opcode);
			break;
		}
		case OpCode::Constants::OP_POP:
		{
			Pop();
			break;
		}
		default:
			throw std::runtime_error("Unknown opcode");
		}
	}
}

std::shared_ptr<IObject> RogueVM::Top() const 
{ 
	return _sp > 0 ? _stack[_sp - 1] : NullObj::NULL_OBJ_REF; 
}
void RogueVM::Push(std::shared_ptr<IObject> obj) 
{
	if (_sp >= _stack.size()) 
	{ 
		throw std::exception("Stack Overflow"); 
	}  
	_stack[_sp++] = obj; 
}
std::shared_ptr<IObject> RogueVM::Pop() 
{ 
	if (_sp == 0) 
	{ 
		throw std::exception("Stack Underflow"); 
	} 
	return _stack[--_sp];
}

std::shared_ptr<IObject> RogueVM::LastPoppped() const 
{ 
	if (_sp == 0)
	{
		if (_stack[_sp] == nullptr)
		{
			return NullObj::NULL_OBJ_REF;
		}
	}
	return _stack[_sp];
}

void RogueVM::ExecuteArithmeticInfix(OpCode::Constants opcode)
{
	auto right = Pop();
	auto left = Pop();
	if (left->Type() != right->Type())
	{
		throw std::runtime_error("Type mismatch");
	}
	if (left->Type() == ObjectType::INTEGER_OBJ)
	{
		ExecuteIntegerArithmeticInfix(opcode, *std::dynamic_pointer_cast<IntegerObj>(left), *std::dynamic_pointer_cast<IntegerObj>(right));
	}
	else if (left->Type() == ObjectType::DECIMAL_OBJ)
	{
		ExecuteDecimalArithmeticInfix(opcode, *std::dynamic_pointer_cast<DecimalObj>(left), *std::dynamic_pointer_cast<DecimalObj>(right));
	}
	else if (left->Type() == ObjectType::STRING_OBJ)
	{
		ExecuteStringArithmeticInfix(opcode, *std::dynamic_pointer_cast<StringObj>(left), *std::dynamic_pointer_cast<StringObj>(right));
	}
	else
	{
		throw std::runtime_error(std::format("ExecuteArithmeticInfix: Unsupported type {}", left->Type().Name));
	}
}

void RogueVM::ExecuteIntegerArithmeticInfix(OpCode::Constants opcode, IntegerObj left, IntegerObj right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_ADD:
	{
		auto result = IntegerObj::New(left.Value + right.Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_SUB:
	{
		auto result = IntegerObj::New(left.Value - right.Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_MUL:
	{
		auto result = IntegerObj::New(left.Value * right.Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_DIV:
	{
		auto result = IntegerObj::New(left.Value / right.Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_MOD:
	{
		auto result = IntegerObj::New(left.Value % right.Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BOR:
	{
		auto result = IntegerObj::New(left.Value | right.Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BAND:
	{
		auto result = IntegerObj::New(left.Value & right.Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BXOR:
	{
		auto result = IntegerObj::New(left.Value ^ right.Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BLSHIFT:
	{
		auto result = IntegerObj::New(left.Value << right.Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BRSHIFT:
	{
		auto result = IntegerObj::New(left.Value >> right.Value);
		Push(result);
		break;
	}
	default:
	{
		throw std::runtime_error(MakeOpCodeError("ExecuteIntegerArithmeticInfix: Unknown opcode", opcode));
	}		
	}
}

void RogueVM::ExecuteDecimalArithmeticInfix(OpCode::Constants opcode, DecimalObj left, DecimalObj right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_ADD:
	{
		auto result = DecimalObj::New(left.Value + right.Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_SUB:
	{
		auto result = DecimalObj::New(left.Value - right.Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_MUL:
	{
		auto result = DecimalObj::New(left.Value * right.Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_DIV:
	{
		auto result = DecimalObj::New(left.Value / right.Value);
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteDecimalArithmeticInfix: Unknown opcode", opcode));
	}
}

void RogueVM::ExecuteStringArithmeticInfix(OpCode::Constants opcode, StringObj left, StringObj right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_ADD:
	{
		auto result = StringObj::New(left.Value + right.Value);
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteStringArithmeticInfix: Unknown opcode", opcode));
	}
}

std::string RogueVM::MakeOpCodeError(const std::string& message, OpCode::Constants opcode)
{
	auto def = OpCode::Lookup(opcode);
	if (std::holds_alternative<std::string>(def))
	{
		return std::format("{} : {}", message, std::get<std::string>(def));
	}
	else
	{
		auto definition = std::get<Definition>(def);
		return std::format("{} : {}",message, definition.Name);
	}
}