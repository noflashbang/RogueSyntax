#include "pch.h"

RogueVM::RogueVM(const ByteCode& byteCode, const std::shared_ptr<ObjectFactory>& factory)
	: _byteCode(byteCode), _externals(nullptr), _factory(factory), _coercer(factory)
{
	//main frame
	auto function = _factory->New<FunctionCompiledObj>(_byteCode.Instructions, 0, 0);
	auto closure = _factory->New<ClosureObj>(function, std::vector<const IObject*>{});
	PushFrame(Frame(closure, 0));
	_externals = nullptr;
}

RogueVM::RogueVM(const ByteCode& byteCode, const std::shared_ptr<BuiltIn>& externals, const std::shared_ptr<ObjectFactory>& factory) : _byteCode(byteCode), _externals(externals), _factory(factory), _coercer(factory)
{
	//main frame
	auto function = _factory->New<FunctionCompiledObj>(_byteCode.Instructions, 0, 0);
	auto closure = _factory->New<ClosureObj>(function, std::vector<const IObject*>{});
	PushFrame(Frame(closure, 0));
}

RogueVM::~RogueVM()
{
}

void RogueVM::Run()
{
	//make a local copy of the constants
	auto pgrmConstants = _byteCode.Constants;	
	std::vector<const IObject*> constants;
	constants.reserve(pgrmConstants.size());
	for (auto& c : pgrmConstants)
	{
		auto copy = _factory->Clone(c);
		constants.push_back(copy);
	}

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

			std::vector<const IObject*> elements;
			elements.reserve(numElements);
			for (int i = 0; i < numElements; i++)
			{
				elements.push_back(Pop());
			}
			std::reverse(elements.begin(), elements.end());
			auto array = _factory->New<ArrayObj>(elements);
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

			auto hash = _factory->New<HashObj>(pairs);
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
			if (condition->IsThisA<BooleanObj>())
			{
				if (condition == BooleanObj::FALSE_OBJ_REF)
				{
					SetFrameIp(pos);
				}
			}
			else
			{
				auto coerced = _coercer.EvalAsBoolean(condition);
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
		case OpCode::Constants::OP_SET:
		{
			auto idx = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);
			ExecuteSetInstruction(idx);
			break;
		}
		case OpCode::Constants::OP_GET:
		{
			auto idx = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);
			ExecuteGetInstruction(idx);
			break;
		}
		case OpCode::Constants::OP_SET_ASSIGN:
		{
			auto idx = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);

			auto rValue = Pop();
			auto indexValue = Pop();
			auto arrValue = Pop();
			if (arrValue->IsThisA<ArrayObj>())
			{
				auto arr = dynamic_cast<const ArrayObj*>(arrValue);
				if (indexValue->IsThisA<IntegerObj>())
				{
					auto index = dynamic_cast<const IntegerObj*>(indexValue);
					if (index->Value >= 0 && index->Value < arr->Elements.size())
					{
						auto arrayClone = _factory->Clone(arr);
						auto arrayObj = dynamic_cast<ArrayObj*>(arrayClone);
						auto rValueClone = _factory->Clone(rValue);
						arrayObj->Elements[index->Value] = rValueClone;
						
						Push(arrayObj);
						ExecuteSetInstruction(idx);
						Push(rValueClone);
					}
					else
					{
						throw std::runtime_error("Index out of bounds");
					}
				}
				else
				{
					throw std::runtime_error("Index must be an integer");
				}
			}
			else if (arrValue->IsThisA<HashObj>())
			{
				auto hashClone = _factory->Clone(arrValue);
				auto hash = dynamic_cast<HashObj*>(hashClone);
				auto key = HashKey{ indexValue->Type(), indexValue->Inspect() };

				auto rValueClone = _factory->Clone(rValue);
				
				auto keyClone = _factory->Clone(indexValue);
				
				auto entry = HashEntry{ keyClone, rValueClone };
				hash->Elements[key] = entry;

				Push(hash);
				ExecuteSetInstruction(idx);
				Push(rValueClone);
			}
			else
			{
				throw std::runtime_error("Can only set assign to arrays or hashes");
			}
			break;
		}
		case OpCode::Constants::OP_INDEX:
		{
			auto index = Pop();
			auto left = Pop();

			ExecuteIndexOperation(left, index);
			
			break;
		}
		
		case OpCode::Constants::OP_CALL:
		{
			auto numArgs = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);

			auto calleeIdx = _sp - 1 - numArgs;
			auto callee = _stack[calleeIdx];
			if (callee->IsThisA<ClosureObj>())
			{
				auto closure = dynamic_cast<const ClosureObj*>(callee);
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
			else if (callee->IsThisA<BuiltInObj>())
			{
				if (_externals == nullptr)
				{
					throw std::runtime_error("No external symbols provided");
				}

				auto builtin = dynamic_cast<const BuiltInObj*>(callee);
				auto args = std::vector<const IObject*>(_stack.begin() + calleeIdx + 1, _stack.begin() + _sp);
				auto fn = builtin->Resolve(_externals);
				auto result = fn(args);
				_sp = calleeIdx;

				if (result != nullptr && !result->IsThisA<VoidObj>())
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

			auto fn = dynamic_cast<const FunctionCompiledObj*>(constants[idx]);
			
			std::vector<const IObject*> free;
			free.reserve(numFree);
			for (int i = 0; i < numFree; i++)
			{
				free[i] = _stack[_sp - numFree + i];
			}
			_sp = _sp - numFree;

			auto closure = _factory->New<ClosureObj>(fn, free);
			Push(closure);
			break;
		}
		case OpCode::Constants::OP_CURRENT_CLOSURE:
		{
			auto closure = CurrentFrame().ClosureRef();
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

const IObject* RogueVM::Top() const 
{ 
	return _sp > 0 ? _stack[_sp - 1] : NullObj::NULL_OBJ_REF; 
}
void RogueVM::Push(const IObject* obj) 
{
	if (_sp >= _stack.size()) 
	{ 
		throw std::exception("Stack Overflow"); 
	}  
	_stack[_sp++] = obj; 
}

const IObject* RogueVM::Pop() 
{ 
	if (_sp == 0) 
	{ 
		throw std::exception("Stack Underflow"); 
	} 
	return _stack[--_sp];
}

const IObject* RogueVM::LastPoppped() const 
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
		if (_coercer.CanCoerceTypes(left, right))
		{
			auto [left_c, right_c] = _coercer.CoerceTypes(left, right);
			Push(left_c);
			Push(right_c);
			ExecuteArithmeticInfix(opcode);
			return;
		}
		else
		{
			throw std::runtime_error(std::format("ExecuteArithmeticInfix: Unsupported types {} {}", left->TypeName(), right->TypeName()));
		}
	}
	if (left->IsThisA<IntegerObj>())
	{
		ExecuteIntegerArithmeticInfix(opcode, dynamic_cast<const IntegerObj*>(left), dynamic_cast<const IntegerObj*>(right));
	}
	else if (left->IsThisA<DecimalObj>())
	{
		ExecuteDecimalArithmeticInfix(opcode, dynamic_cast<const DecimalObj*>(left), dynamic_cast<const DecimalObj*>(right));
	}
	else if (left->IsThisA<StringObj>())
	{
		ExecuteStringArithmeticInfix(opcode, dynamic_cast<const StringObj*>(left), dynamic_cast<const StringObj*>(right));
	}
	else
	{
		throw std::runtime_error(std::format("ExecuteArithmeticInfix: Unsupported type {}", left->TypeName()));
	}
}

void RogueVM::ExecuteIntegerArithmeticInfix(OpCode::Constants opcode, const IntegerObj* left, const IntegerObj* right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_ADD:
	{
		auto result = _factory->New<IntegerObj>(left->Value + right->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_SUB:
	{
		auto result = _factory->New<IntegerObj>(left->Value - right->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_MUL:
	{
		auto result = _factory->New<IntegerObj>(left->Value * right->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_DIV:
	{
		auto result = _factory->New<IntegerObj>(left->Value / right->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_MOD:
	{
		auto result = _factory->New<IntegerObj>(left->Value % right->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BOR:
	{
		auto result = _factory->New<IntegerObj>(left->Value | right->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BAND:
	{
		auto result = _factory->New<IntegerObj>(left->Value & right->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BXOR:
	{
		auto result = _factory->New<IntegerObj>(left->Value ^ right->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BLSHIFT:
	{
		auto result = _factory->New<IntegerObj>(left->Value << right->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BRSHIFT:
	{
		auto result = _factory->New<IntegerObj>(left->Value >> right->Value);
		Push(result);
		break;
	}
	default:
	{
		throw std::runtime_error(MakeOpCodeError("ExecuteIntegerArithmeticInfix: Unknown opcode", opcode));
	}		
	}
}

void RogueVM::ExecuteDecimalArithmeticInfix(OpCode::Constants opcode, const DecimalObj* left, const DecimalObj* right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_ADD:
	{
		auto result = _factory->New<DecimalObj>(left->Value + right->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_SUB:
	{
		auto result = _factory->New<DecimalObj>(left->Value - right->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_MUL:
	{
		auto result = _factory->New<DecimalObj>(left->Value * right->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_DIV:
	{
		auto result = _factory->New<DecimalObj>(left->Value / right->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_MOD:
	{
		auto result = _factory->New<DecimalObj>(std::fmod(left->Value, right->Value));
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteDecimalArithmeticInfix: Unknown opcode", opcode));
	}
}

void RogueVM::ExecuteStringArithmeticInfix(OpCode::Constants opcode, const StringObj* left, const StringObj* right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_ADD:
	{
		auto result = _factory->New<StringObj>(left->Value + right->Value);
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
	if (left->IsThisA<IntegerObj>())
	{
		ExecuteIntegerComparisonInfix(opcode, dynamic_cast<const IntegerObj*>(left), dynamic_cast<const IntegerObj*>(right));
	}
	else if (left->IsThisA<DecimalObj>())
	{
		ExecuteDecimalComparisonInfix(opcode, dynamic_cast<const DecimalObj*>(left), dynamic_cast<const DecimalObj*>(right));
	}
	else if (left->IsThisA<StringObj>())
	{
		ExecuteStringComparisonInfix(opcode, dynamic_cast<const StringObj*>(left), dynamic_cast<const StringObj*>(right));
	}
	else if (left->IsThisA<BooleanObj>())
	{
		ExecuteBooleanComparisonInfix(opcode, dynamic_cast<const BooleanObj*>(left), dynamic_cast<const BooleanObj*>(right));
	}
	else if (left->IsThisA<NullObj>())
	{
		ExecuteNullComparisonInfix(opcode, dynamic_cast<const NullObj*>(left), dynamic_cast<const NullObj*>(right));
	}
	else
	{
		throw std::runtime_error(std::format("ExecuteComparisonInfix: Unsupported type {}", left->TypeName()));
	}
}

void RogueVM::ExecuteIntegerComparisonInfix(OpCode::Constants opcode, const IntegerObj* left, const IntegerObj* right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_EQUAL:
	{
		auto result = left->Value == right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NOT_EQUAL:
	{
		auto result = left->Value != right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_GREATER_THAN:
	{
		auto result = left->Value > right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_GREATER_THAN_EQUAL:
	{
		auto result = left->Value >= right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_LESS_THAN:
	{
		auto result = left->Value < right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_LESS_THAN_EQUAL:
	{
		auto result = left->Value <= right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteIntegerComparisonInfix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecuteDecimalComparisonInfix(OpCode::Constants opcode, const DecimalObj* left, const DecimalObj* right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_EQUAL:
	{
		auto result = std::abs(left->Value - right->Value) <= FLT_EPSILON ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NOT_EQUAL:
	{
		auto result = std::abs(left->Value - right->Value) > FLT_EPSILON ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_GREATER_THAN:
	{
		auto result = left->Value > right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_GREATER_THAN_EQUAL:
	{
		auto result = left->Value >= right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_LESS_THAN:
	{
		auto result = left->Value < right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_LESS_THAN_EQUAL:
	{
		auto result = left->Value <= right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteDecimalComparisonInfix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecuteStringComparisonInfix(OpCode::Constants opcode, const StringObj* left, const StringObj* right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_EQUAL:
	{
		auto result = left->Value == right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NOT_EQUAL:
	{
		auto result = left->Value != right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteStringComparisonInfix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecuteNullComparisonInfix(OpCode::Constants opcode, const NullObj* left, const NullObj* right)
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

void RogueVM::ExecuteBooleanComparisonInfix(OpCode::Constants opcode, const BooleanObj* left, const BooleanObj* right)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_EQUAL:
	{
		auto result = left->Value == right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NOT_EQUAL:
	{
		auto result = left->Value != right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BOOL_AND:
	{
		auto result = left->Value && right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BOOL_OR:
	{
		auto result = left->Value || right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
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
	if (right->IsThisA<IntegerObj>())
	{
		ExecuteIntegerPrefix(opcode, dynamic_cast<const IntegerObj*>(right));
	}
	else if (right->IsThisA<DecimalObj>())
	{
		ExecuteDecimalPrefix(opcode, dynamic_cast<const DecimalObj*>(right));
	}
	else if (right->IsThisA<BooleanObj>())
	{
		ExecuteBooleanPrefix(opcode, dynamic_cast<const BooleanObj*>(right));
	}
	else if (right->IsThisA<NullObj>())
	{
		ExecuteNullPrefix(opcode, dynamic_cast<const NullObj*>(right));
	}
	else
	{
		throw std::runtime_error(std::format("ExecutePrefix: Unsupported type {}", right->TypeName()));
	}
}

void RogueVM::ExecuteIntegerPrefix(OpCode::Constants opcode, const IntegerObj* obj)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_NEGATE:
	{
		auto result = _factory->New<IntegerObj>(-obj->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NOT:
	{
		auto result = obj->Value == 0 ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_BNOT:
	{
		auto result = _factory->New<IntegerObj>(~obj->Value);
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteIntegerPrefix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecuteDecimalPrefix(OpCode::Constants opcode, const DecimalObj* obj)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_NEGATE:
	{
		auto result = _factory->New<DecimalObj>(-obj->Value);
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NOT:
	{
		auto result = std::abs(obj->Value) < FLT_EPSILON ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteDecimalPrefix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecuteBooleanPrefix(OpCode::Constants opcode, const BooleanObj* obj)
{
	switch (opcode)
	{
	case OpCode::Constants::OP_NOT:
	{
		auto result = obj->Value ? BooleanObj::FALSE_OBJ_REF : BooleanObj::TRUE_OBJ_REF;
		Push(result);
		break;
	}
	default:
		throw std::runtime_error(MakeOpCodeError("ExecuteBooleanPrefix: Unsupported opcode", opcode));
	}
}

void RogueVM::ExecuteNullPrefix(OpCode::Constants opcode, const NullObj* obj)
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

void RogueVM::ExecuteIndexOperation(const IObject* left, const IObject* index)
{
	if (left->IsThisA<ArrayObj>())
	{
		auto arr = dynamic_cast<const ArrayObj*>(left);
		auto idx = dynamic_cast<const IntegerObj*>(index);
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
	else if (left->IsThisA<HashObj>())
	{
		const IObject* result = NullObj::NULL_OBJ_REF;
		auto hash = dynamic_cast<const HashObj*>(left);
		auto key = HashKey{ index->Type(), index->Inspect() };
		auto entry = hash->Elements.find(key);
		if (entry != hash->Elements.end())
		{
			result = entry->second.Value;
		}
		Push(result);
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

void RogueVM::ExecuteGetInstruction(int idx)
{
	auto type = GetTypeFromIdx(idx);
	switch (type)
	{
		case GetSetType::GLOBAL:
		{
			auto adjustedIdx = (idx & 0x3FFF);
			auto global = _globals[adjustedIdx];
			Push(global);
			break;
		}
		case GetSetType::LOCAL:
		{
			auto adjustedIdx = (idx & 0x3FFF);
			auto local = CurrentFrame().BasePointer() + adjustedIdx;
			Push(_stack[adjustedIdx]);
			break;
		}
		case GetSetType::EXTERN:
		{
			auto adjustedIdx = (idx & 0x3FFF);
			Push(_factory->New<BuiltInObj>(adjustedIdx));
			break;
		}
		case GetSetType::FREE:
		{
			auto adjustedIdx = (idx & 0x3FFF);
			auto free = CurrentFrame().Closure()->Frees[adjustedIdx];
			Push(free);
			break;
		}
	}
}

void RogueVM::ExecuteSetInstruction(int idx)
{
	auto type = GetTypeFromIdx(idx);
	switch (type)
	{
		case GetSetType::GLOBAL:
		{
			auto adjustedIdx = (idx & 0x3FFF);
			auto global = Pop();
			auto cloned = _factory->Clone(global);
			_globals[adjustedIdx] = cloned;
			break;
		}
		case GetSetType::LOCAL:
		{
			auto adjustedIdx = (idx & 0x3FFF);
			auto local = Pop();
			auto localIdx = CurrentFrame().BasePointer() + idx;
			auto cloned =  _factory->Clone(local);
			_stack[adjustedIdx] = cloned;
			break;
		}
	}
}

GetSetType RogueVM::GetTypeFromIdx(int idx)
{
	if ((idx | 0x3FFF) == 0x3FFF)
	{
		return GetSetType::LOCAL;
	}
	else if ((idx & 0x8000) != 0)
	{
		return GetSetType::GLOBAL;
	}
	else if ((idx & 0x4000) != 0)
	{
		return GetSetType::EXTERN;
	}
	else if ((idx & 0xC000) != 0)
	{
		return GetSetType::FREE;
	}
	else
	{
		throw std::runtime_error("Invalid index");
	}
}

