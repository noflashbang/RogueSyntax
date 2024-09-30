#include "VirtualMachine.h"
#include "pch.h"

RogueVM::RogueVM(const ByteCode& byteCode)
	: _byteCode(byteCode)
{
	//main frame
	PushFrame(Frame(ClosureObj::New(FunctionCompiledObj::New(_byteCode.Instructions, 0, 0), {}), 0));
	_externals = nullptr;
}

RogueVM::RogueVM(const ByteCode& byteCode, std::shared_ptr<BuiltIn> externals) : _byteCode(byteCode), _externals(externals)
{
	//main frame
	PushFrame(Frame(ClosureObj::New(FunctionCompiledObj::New(_byteCode.Instructions, 0, 0), {}), 0));
}

RogueVM::~RogueVM()
{
}

void RogueVM::Run()
{
	auto constants = _byteCode.Constants;	
	while (CurrentFrame().Ip() < CurrentFrame().Instructions().size())
	{
		auto instructions = CurrentFrame().Instructions();
		auto opcode = OpCode::GetOpcode(instructions, CurrentFrame().Ip());
		IncrementFrameIp();

		switch (opcode)
		{
		case OpCode::Constants::OP_CONSTANT:
		{
			auto idx = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			auto constant = constants[idx];
			//std::cout << constant->Inspect() << std::endl;
			IncrementFrameIp(2);

			Push(constant);
			break;
		}
		case OpCode::Constants::OP_ARRAY:
		{
			auto numElements = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);

			std::vector<std::shared_ptr<IObject>> elements;
			for (int i = 0; i < numElements; i++)
			{
				elements.push_back(Pop());
			}
			std::reverse(elements.begin(), elements.end());
			auto array = ArrayObj::New(elements);
			Push(array);
			break;
		}
		case OpCode::Constants::OP_HASH:
		{
			auto numElements = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);

			std::unordered_map<HashKey, HashEntry> pairs;
			for (int i = 0; i < numElements; i++)
			{
				auto value = Pop();
				auto key = Pop();

				pairs[HashKey{ key->Type(), key->Inspect() }] = HashEntry{ key, value };
			}

			auto hash = HashObj::New(pairs);
			Push(hash);
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
		case OpCode::Constants::OP_EQUAL:
		case OpCode::Constants::OP_NOT_EQUAL:
		case OpCode::Constants::OP_GREATER_THAN:
		case OpCode::Constants::OP_GREATER_THAN_EQUAL:
		case OpCode::Constants::OP_LESS_THAN:
		case OpCode::Constants::OP_LESS_THAN_EQUAL:
		case OpCode::Constants::OP_BOOL_AND:
		case OpCode::Constants::OP_BOOL_OR:
		{
			ExecuteComparisonInfix(opcode);
			break;
		}
		case OpCode::Constants::OP_NEGATE:
		case OpCode::Constants::OP_NOT:
		case OpCode::Constants::OP_BNOT:
		{
			ExecutePrefix(opcode);
			break;
		}
		case OpCode::Constants::OP_POP:
		{
			Pop();
			break;
		}
		case OpCode::Constants::OP_TRUE:
		{
			Push(BooleanObj::TRUE_OBJ_REF);
			break;
		}
		case OpCode::Constants::OP_FALSE:
		{
			Push(BooleanObj::FALSE_OBJ_REF);
			break;
		}
		case OpCode::Constants::OP_JUMP:
		{
			auto pos = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			SetFrameIp(pos);
			break;
		}
		case OpCode::Constants::OP_JUMP_IF_FALSE:
		{
			auto pos = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);
			auto condition = Pop();
			if (condition->Type() == ObjectType::BOOLEAN_OBJ)
			{
				if (condition == BooleanObj::FALSE_OBJ_REF)
				{
					SetFrameIp(pos);
				}
			}
			else
			{
				auto coerced = _coercer.EvalAsBoolean(condition.get());
				if (coerced == BooleanObj::FALSE_OBJ_REF)
				{
					SetFrameIp(pos);
				}
			}
			break;
		}
		case OpCode::Constants::OP_NULL:
		{
			Push(NullObj::NULL_OBJ_REF);
			break;
		}
		case OpCode::Constants::OP_GET_GLOBAL:
		{
			auto idx = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);
			auto global = _globals[idx];
			Push(global);
			break;
		}
		case OpCode::Constants::OP_SET_GLOBAL:
		{
			auto idx = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);
			auto global = Pop();
			_globals[idx] = global;
			break;
		}
		case OpCode::Constants::OP_GET_LOCAL:
		{
			auto idx = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);
			auto local = CurrentFrame().BasePointer() + idx;
			Push(_stack[local]);
			break;
		}
		case OpCode::Constants::OP_SET_LOCAL:
		{
			auto idx = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);
			auto local = Pop();
			auto localIdx = CurrentFrame().BasePointer() + idx;
			_stack[localIdx] = local;
			break;
		}
		case OpCode::Constants::OP_INDEX:
		{
			auto index = Pop();
			auto left = Pop();

			ExecuteIndexOperation(left.get(), index.get());
			
			break;
		}
		case OpCode::Constants::OP_GET_EXTRN:
		{
			auto idx = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);
			Push(BuiltInObj::New(idx));
			break;
		}
		case OpCode::Constants::OP_GET_FREE:
		{
			auto idx = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);
			auto free = CurrentFrame().Closure()->Frees[idx];
			Push(free);
			break;
		}
		case OpCode::Constants::OP_CALL:
		{
			auto numArgs = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);

			auto calleeIdx = _sp - 1 - numArgs;
			auto callee = _stack[calleeIdx];
			if (callee->Type() == ObjectType::CLOSURE_OBJ)
			{
				auto closure = std::dynamic_pointer_cast<ClosureObj>(callee);
				auto fn = closure->Function;
				if (numArgs != fn->NumParameters)
				{
					throw std::runtime_error(std::format("Expected {} arguments but got {}", fn->NumParameters, numArgs));
				}

				auto frame = Frame(closure, _sp - numArgs);
				PushFrame(frame);
				//make room for locals
				_sp = frame.BasePointer() + fn->NumLocals;
			}
			else if (callee->Type() == ObjectType::BUILTIN_OBJ)
			{
				if (_externals == nullptr)
				{
					throw std::runtime_error("No external symbols provided");
				}

				auto builtin = std::dynamic_pointer_cast<BuiltInObj>(callee);
				auto args = std::vector<std::shared_ptr<IObject>>(_stack.begin() + calleeIdx + 1, _stack.begin() + _sp);
				auto fn = builtin->Resolve(_externals);
				auto result = fn(args);
				_sp = calleeIdx;

				if (result != nullptr && result->Type() != ObjectType::VOID_OBJ)
				{
					Push(result);
				}
			}
			else
			{
				throw std::runtime_error("Can only call functions or externals");
			}
			break;
		}
		case OpCode::Constants::OP_CLOSURE:
		{
			auto idx = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);
			auto numFree = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);

			auto fn = std::dynamic_pointer_cast<FunctionCompiledObj>(constants[idx]);
			
			std::vector<std::shared_ptr<IObject>> free(numFree);
			for (int i = 0; i < numFree; i++)
			{
				free[i] = _stack[_sp - numFree + i];
			}
			_sp = _sp - numFree;

			auto closure = ClosureObj::New(fn, free);
			Push(closure);
			break;
		}
		case OpCode::Constants::OP_RETURN:
		{
			auto frame = PopFrame();
			_sp = frame.BasePointer();
			Pop();
			break;
		}
		case OpCode::Constants::OP_RETURN_VALUE:
		{
			auto result = Pop();
			auto frame = PopFrame();
			_sp = frame.BasePointer();
			Pop();
			Push(result);
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

const Frame& RogueVM::CurrentFrame() const
{
	if (_frameIndex == 0)
	{
		throw std::exception("No frames");
	}
	return _frames[_frameIndex - 1];
}

void RogueVM::PushFrame(Frame frame)
{
	if (_frameIndex >= MAX_FRAMES)
	{
		throw std::exception("Frame stack overflow");
	}
	_frames[_frameIndex++] = frame;
}

Frame RogueVM::PopFrame()
{
	if (_frameIndex == 0)
	{
		throw std::exception("Frame stack underflow");
	}
	return _frames[--_frameIndex];
}

void RogueVM::ExecuteArithmeticInfix(OpCode::Constants opcode)
{
	auto right = Pop();
	auto left = Pop();
	if (left->Type() != right->Type())
	{
		if (_coercer.CanCoerceTypes(left.get(), right.get()))
		{
			auto [left_c, right_c] = _coercer.CoerceTypes(left.get(), right.get());
			Push(left_c);
			Push(right_c);
			ExecuteArithmeticInfix(opcode);
			return;
		}
		else
		{
			throw std::runtime_error(std::format("ExecuteArithmeticInfix: Unsupported types {} {}", left->Type().Name, right->Type().Name));
		}
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
	case OpCode::Constants::OP_MOD:
	{
		auto result = DecimalObj::New(std::fmod(left.Value, right.Value));
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

void RogueVM::ExecuteComparisonInfix(OpCode::Constants opcode)
{
	auto right = Pop();
	auto left = Pop();
	if (left->Type() != right->Type())
	{
		throw std::runtime_error("Type mismatch");
	}
	if (left->Type() == ObjectType::INTEGER_OBJ)
	{
		ExecuteIntegerComparisonInfix(opcode, *std::dynamic_pointer_cast<IntegerObj>(left), *std::dynamic_pointer_cast<IntegerObj>(right));
	}
	else if (left->Type() == ObjectType::DECIMAL_OBJ)
	{
		ExecuteDecimalComparisonInfix(opcode, *std::dynamic_pointer_cast<DecimalObj>(left), *std::dynamic_pointer_cast<DecimalObj>(right));
	}
	else if (left->Type() == ObjectType::STRING_OBJ)
	{
		ExecuteStringComparisonInfix(opcode, *std::dynamic_pointer_cast<StringObj>(left), *std::dynamic_pointer_cast<StringObj>(right));
	}
	else if (left->Type() == ObjectType::BOOLEAN_OBJ)
	{
		ExecuteBooleanComparisonInfix(opcode, *std::dynamic_pointer_cast<BooleanObj>(left), *std::dynamic_pointer_cast<BooleanObj>(right));
	}
	else if (left->Type() == ObjectType::NULL_OBJ)
	{
		ExecuteNullComparisonInfix(opcode, *std::dynamic_pointer_cast<NullObj>(left), *std::dynamic_pointer_cast<NullObj>(right));
	}
	else
	{
		throw std::runtime_error(std::format("ExecuteComparisonInfix: Unsupported type {}", left->Type().Name));
	}
}

void RogueVM::ExecuteIntegerComparisonInfix(OpCode::Constants opcode, IntegerObj left, IntegerObj right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_EQUAL:
	{
		auto result = left.Value == right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NOT_EQUAL:
	{
		auto result = left.Value != right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_GREATER_THAN:
	{
		auto result = left.Value > right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_GREATER_THAN_EQUAL:
	{
		auto result = left.Value >= right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_LESS_THAN:
	{
		auto result = left.Value < right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_LESS_THAN_EQUAL:
	{
		auto result = left.Value <= right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteIntegerComparisonInfix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecuteDecimalComparisonInfix(OpCode::Constants opcode, DecimalObj left, DecimalObj right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_EQUAL:
	{
		auto result = std::abs(left.Value - right.Value) <= FLT_EPSILON ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NOT_EQUAL:
	{
		auto result = std::abs(left.Value - right.Value) > FLT_EPSILON ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_GREATER_THAN:
	{
		auto result = left.Value > right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_GREATER_THAN_EQUAL:
	{
		auto result = left.Value >= right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_LESS_THAN:
	{
		auto result = left.Value < right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_LESS_THAN_EQUAL:
	{
		auto result = left.Value <= right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteDecimalComparisonInfix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecuteStringComparisonInfix(OpCode::Constants opcode, StringObj left, StringObj right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_EQUAL:
	{
		auto result = left.Value == right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NOT_EQUAL:
	{
		auto result = left.Value != right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteStringComparisonInfix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecuteNullComparisonInfix(OpCode::Constants opcode, NullObj left, NullObj right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_EQUAL:
	{
		Push(BooleanObj::TRUE_OBJ_REF);
		break;
	}
	case OpCode::Constants::OP_NOT_EQUAL:
	{
		Push(BooleanObj::FALSE_OBJ_REF);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteNullComparisonInfix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecuteBooleanComparisonInfix(OpCode::Constants opcode, BooleanObj left, BooleanObj right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_EQUAL:
	{
		auto result = left.Value == right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NOT_EQUAL:
	{
		auto result = left.Value != right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BOOL_AND:
	{
		auto result = left.Value && right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BOOL_OR:
	{
		auto result = left.Value || right.Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteBooleanComparisonInfix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecutePrefix(OpCode::Constants opcode)
{
	auto right = Pop();
	if (right->Type() == ObjectType::INTEGER_OBJ)
	{
		ExecuteIntegerPrefix(opcode, *std::dynamic_pointer_cast<IntegerObj>(right));
	}
	else if (right->Type() == ObjectType::DECIMAL_OBJ)
	{
		ExecuteDecimalPrefix(opcode, *std::dynamic_pointer_cast<DecimalObj>(right));
	}
	else if (right->Type() == ObjectType::BOOLEAN_OBJ)
	{
		ExecuteBooleanPrefix(opcode, *std::dynamic_pointer_cast<BooleanObj>(right));
	}
	else if (right->Type() == ObjectType::NULL_OBJ)
	{
		ExecuteNullPrefix(opcode, *std::dynamic_pointer_cast<NullObj>(right));
	}
	else
	{
		throw std::runtime_error(std::format("ExecutePrefix: Unsupported type {}", right->Type().Name));
	}
}

void RogueVM::ExecuteIntegerPrefix(OpCode::Constants opcode, IntegerObj obj)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_NEGATE:
	{
		auto result = IntegerObj::New(-obj.Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NOT:
	{
		auto result = obj.Value == 0 ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BNOT:
	{
		auto result = IntegerObj::New(~obj.Value);
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteIntegerPrefix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecuteDecimalPrefix(OpCode::Constants opcode, DecimalObj obj)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_NEGATE:
	{
		auto result = DecimalObj::New(-obj.Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NOT:
	{
		auto result = std::abs(obj.Value) < FLT_EPSILON ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteDecimalPrefix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecuteBooleanPrefix(OpCode::Constants opcode, BooleanObj obj)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_NOT:
	{
		auto result = obj.Value ? BooleanObj::FALSE_OBJ_REF : BooleanObj::TRUE_OBJ_REF;
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteBooleanPrefix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecuteNullPrefix(OpCode::Constants opcode, NullObj obj)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_NOT:
	{
		Push(BooleanObj::TRUE_OBJ_REF);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteNullPrefix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecuteIndexOperation(IObject* left, IObject* index)
{
	if (left->Type() == ObjectType::ARRAY_OBJ)
	{
		auto arr = dynamic_cast<ArrayObj*>(left);
		auto idx = dynamic_cast<IntegerObj*>(index);
		if (idx == nullptr)
		{
			throw std::runtime_error("Index must be an integer");
		}
		if (idx->Value < 0 || idx->Value >= arr->Elements.size())
		{
			throw std::runtime_error("Index out of bounds");
		}
		auto value = arr->Elements[idx->Value];
		Push(value);
	}
	else if (left->Type() == ObjectType::HASH_OBJ)
	{
		auto hash = dynamic_cast<HashObj*>(left);
		auto key = HashKey{ index->Type(), index->Inspect() };
		auto entry = hash->Elements[key];
		Push(entry.Value);
	}
	else
	{
		throw std::runtime_error("Index operation not supported");
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

