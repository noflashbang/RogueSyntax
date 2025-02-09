#include "VirtualMachine.h"
#include "VirtualMachine.h"
#include "VirtualMachine.h"
#include "VirtualMachine.h"
#include "VirtualMachine.h"
#include "VirtualMachine.h"
#include "VirtualMachine.h"
#include "VirtualMachine.h"
#include "VirtualMachine.h"
#include "pch.h"


std::string StackTrace::ToString() const
{
	std::string info = "";
	for (size_t i = 0; i < Frames.size(); i++)
	{
		info += std::format("Frame: {:0>2}, Offset:@{:0>4}+{:0>4} -> |{:0>4}|\n", i, Frames[i].BaseInstructionOffset, Frames[i].FrameInstructionOffset, Frames[i].AbsoluteInstructionOffest);
	}
	return info;
};

std::string RogueVm_RuntimeError::ToString() const
{
	return std::format("Runtime Error: {}\n{}", Message, StackTrace.ToString());
};

RogueVM::RogueVM(const ByteCode& byteCode, const std::shared_ptr<ObjectFactory>& factory)
	: _byteCode(byteCode), _externals(nullptr), _factory(factory), _coercer(factory), _onError(std::bind(&RogueVM::OnErrorInternal, this, std::placeholders::_1)), _onBreak(std::bind(&RogueVM::OnBreakInternal, this, std::placeholders::_1))
{
	//main frame
	auto function = _factory->New<FunctionCompiledObj>(_byteCode.Instructions, 0, 0);
	auto closure = _factory->New<ClosureObj>(function, std::vector<const IObject*>{});
	PushFrame(Frame(closure, 0));
	_externals = nullptr;
}

RogueVM::RogueVM(const ByteCode& byteCode, const std::shared_ptr<BuiltIn>& externals, const std::shared_ptr<ObjectFactory>& factory) : _byteCode(byteCode), _externals(externals), _factory(factory), _coercer(factory), _onError(std::bind(&RogueVM::OnErrorInternal, this, std::placeholders::_1)), _onBreak(std::bind(&RogueVM::OnBreakInternal, this, std::placeholders::_1))
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
	bool hadError = false;
	RogueVm_RuntimeError error;
	try
	{
		Execute();
	}
	catch (const RogueVm_RuntimeError& ex)
	{
		hadError = true;
		error = ex;;
	}
	if (hadError)
	{
		_onError(error);
	}
}

void RogueVM::Set_RTI_ErrorCallback(const std::function<void(const RogueVm_RuntimeError&)>& onError)
{
	_onError = onError;
}

void RogueVM::Set_RTI_BreakCallback(const std::function<void(const StackTrace&)>& onBreak)
{
	_onBreak = onBreak;
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
	_outputRegister =  _stack[--_sp];
	return _outputRegister;
}

const IObject* RogueVM::LastPopped() const 
{ 
	return _outputRegister;
}

const Frame& RogueVM::CurrentFrame() const
{
	if (_frameIndex == 0)
	{
		throw std::exception("No frames");
	}
	return _frames[_frameIndex - 1];
}

void RogueVM::OnErrorInternal(const RogueVm_RuntimeError& error)
{
	_outputRegister = _factory->New<StringObj>(error.ToString());
}

void RogueVM::OnBreakInternal(const StackTrace& stack)
{
}

FrameTrace RogueVM::GetFrameTrace(const size_t idx, const Frame& frame) const
{
	FrameTrace trace;
	auto frameidx = idx;
	if (frameidx == 0)
	{
		auto baseOffset = 0;
		auto baseStackPointer = 0;
		auto baseClosure = frame.Closure();

		if (baseClosure != nullptr)
		{
			auto fn = baseClosure->Function;
			if (fn != nullptr)
			{
				baseOffset = fn->FuncOffset;
			}
		}
		auto baseAdjust = baseOffset;
		auto ipAdjust = frame.BeforeIp();

		trace.FrameIdx = frameidx;
		trace.AbsoluteInstructionOffest = baseAdjust + ipAdjust;
		trace.BaseInstructionOffset = baseAdjust;
		trace.FrameInstructionOffset = ipAdjust;

		auto i = 0;
		while (_globals[i] != nullptr)
		{
			StackValue value;
			value.Type = _globals[i]->TypeName();
			value.Value = _globals[i]->Inspect();
			trace.Stack.push_back(value);
			i++;
		}
	}
	else
	{
		auto baseOffset = 0;
		auto baseStackPointer = 0;
		auto baseClosure = frame.Closure();
		auto locals = 0;
		if (baseClosure != nullptr)
		{
			auto fn = baseClosure->Function;
			if (fn != nullptr)
			{
				baseOffset = fn->FuncOffset;
				locals = fn->NumLocals;
			}
		}
		auto baseAdjust = baseOffset >= 9 ? baseOffset - 9 : baseOffset; //9 is the number of bytes for the func instruction - point to the function rather than the first instruction in the function
		auto ipAdjust = frame.BeforeIp() + 9; //add back the 9 bytes to get the correct offset

		trace.FrameIdx = frameidx;
		trace.AbsoluteInstructionOffest = baseAdjust + ipAdjust;
		trace.BaseInstructionOffset = baseAdjust;
		trace.FrameInstructionOffset = ipAdjust;

		for (int i = frame.BasePointer(); i < frame.BasePointer() + locals; i++)
		{
			if (_stack[i] != nullptr)
			{
				StackValue value;
				value.Type = _stack[i]->TypeName();
				value.Value = _stack[i]->Inspect();
				trace.Stack.push_back(value);
			}
		}
	}

	return trace;
}

StackTrace RogueVM::GetRuntimeInfo() const
{
	StackTrace trace;
	auto frameidx = 0;
	while (frameidx < _frameIndex)
	{
		trace.Frames.push_back(GetFrameTrace(frameidx, _frames[frameidx]));
		frameidx++;
	}
	return trace;
}

std::string RogueVM::PrintStack() const
{
	std::string stack;
	for (int i = 0; i < _sp; i++)
	{
		if (i > 0)
		{
			if (_stack[i] != nullptr)
			{
				stack += std::format("{:0>2} : {}\n", i, _stack[i]->Inspect());
			}
			else
			{
				stack += std::format("{:0>2} : NULL\n", i);
			}
		}
	}
	return stack;
}

void RogueVM::Execute()
{
	while (CurrentFrame().Ip() < CurrentFrame().Instructions().size())
	{
		const auto& instructions = CurrentFrame().Instructions();
		CurrentFrame().SaveBeforeIp(); //save the ip before the instruction is executed
		auto opcode = OpCode::GetOpcode(instructions, CurrentFrame().Ip());
		IncrementFrameIp();
		
		switch (opcode)
		{
		case OpCode::Constants::OP_LINT:
		{
			auto value = ReadOperand<int>(4);
			auto integer = _factory->New<IntegerObj>(value);
			Push(integer);
			break;
		}
		case OpCode::Constants::OP_LDECIMAL:
		{
			auto value = ReadOperand<int>(4);
			// reinterpret the value as a float
			float f = reinterpret_cast<float&>(value);
			auto decimal = _factory->New<DecimalObj>(f);
			Push(decimal);
			break;
		}
		case OpCode::Constants::OP_LSTRING:
		{
			auto strLen = ReadOperand<int>(4);
			std::string str;
			for (int i = 0; i < strLen; i++)
			{
				str.push_back(instructions[CurrentFrame().Ip()]);
				IncrementFrameIp(1);
			}
			auto string = _factory->New<StringObj>(str);
			Push(string);
			break;
		}
		case OpCode::Constants::OP_LFUN:
		{
			auto numLocals = ReadOperand<int>(2);
			auto numParameters = ReadOperand<int>(2);
			auto numInstructions = ReadOperand<int>(4);

			std::vector<uint8_t> fnInstructions;
			for (int i = 0; i < numInstructions; i++)
			{
				fnInstructions.push_back(instructions[CurrentFrame().Ip()]);
				IncrementFrameIp(1);
			}
			auto function = _factory->New<FunctionCompiledObj>(fnInstructions, numLocals, numParameters);
			function->FuncOffset = CurrentFrame().BaseOffset() + CurrentFrame().Ip() - numInstructions;
			Push(function);
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
		case OpCode::Constants::OP_EQ:
		case OpCode::Constants::OP_NEQ:
		case OpCode::Constants::OP_GT:
		case OpCode::Constants::OP_GTE:
		case OpCode::Constants::OP_LT:
		case OpCode::Constants::OP_LTE:
		case OpCode::Constants::OP_AND:
		case OpCode::Constants::OP_OR:
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
		case OpCode::Constants::OP_JUMPIFZ:
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
						auto rti = GetRuntimeInfo();
						throw RogueVm_RuntimeError{ std::format("Index out of bounds value[{}] > {}", index->Value, arr->Elements.size()), rti };
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
				Push(result);
			}
			else
			{
				throw std::runtime_error("Can only call functions or externals");
			}
			break;
		}
		case OpCode::Constants::OP_CLOSURE:
		{
			//auto idx = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			//IncrementFrameIp(2);
			auto numFree = instructions[CurrentFrame().Ip()] << 8 | instructions[CurrentFrame().Ip() + 1];
			IncrementFrameIp(2);

			//auto fn = dynamic_cast<const FunctionCompiledObj*>(constants[idx]);
			auto fn = dynamic_cast<const FunctionCompiledObj*>(Pop());

			std::vector<const IObject*> free;
			free.reserve(numFree);
			for (int i = 0; i < numFree; i++)
			{
				free.push_back(_stack[_sp - numFree + i]);
			}
			_sp = _sp - numFree;

			auto closure = _factory->New<ClosureObj>(fn, free);
			Push(closure);
			break;
		}
		case OpCode::Constants::OP_CUR_CLOSURE:
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
		case OpCode::Constants::OP_RET_VAL:
		{
			auto result = Pop();
			if (_frameIndex <= 1)
			{
				//global frame - load result into output
				_outputRegister = result;
			}
			else
			{
				auto frame = PopFrame();
				_sp = frame.BasePointer();
				Pop();
				Push(result);
			}
			break;
		}
		default:
			throw std::runtime_error("Unknown opcode");
		}
	}
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
	case OpCode::Constants::OP_EQ:
	{
		auto result = left->Value == right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NEQ:
	{
		auto result = left->Value != right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_GT:
	{
		auto result = left->Value > right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_GTE:
	{
		auto result = left->Value >= right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_LT:
	{
		auto result = left->Value < right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_LTE:
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
	case OpCode::Constants::OP_EQ:
	{
		auto result = std::abs(left->Value - right->Value) <= FLT_EPSILON ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NEQ:
	{
		auto result = std::abs(left->Value - right->Value) > FLT_EPSILON ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_GT:
	{
		auto result = left->Value > right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_GTE:
	{
		auto result = left->Value >= right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_LT:
	{
		auto result = left->Value < right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_LTE:
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
	case OpCode::Constants::OP_EQ:
	{
		auto result = left->Value == right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NEQ:
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
	case OpCode::Constants::OP_EQ:
	{
		Push(BooleanObj::TRUE_OBJ_REF);
		break;
	}
	case OpCode::Constants::OP_NEQ:
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
	case OpCode::Constants::OP_EQ:
	{
		auto result = left->Value == right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_NEQ:
	{
		auto result = left->Value != right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_AND:
	{
		auto result = left->Value && right->Value ? BooleanObj::TRUE_OBJ_REF : BooleanObj::FALSE_OBJ_REF;
		Push(result);
		break;
	}
	case OpCode::Constants::OP_OR:
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
			auto rti = GetRuntimeInfo();
			throw RogueVm_RuntimeError{ std::format("Index out of bounds value[{}] > {}", idx->Value, arr->Elements.size()), rti };
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
		case ScopeType::SCOPE_GLOBAL:
		{
			auto adjustedIdx = AdjustIdx(idx);
			auto global = _globals[adjustedIdx];
			Push(global);
			break;
		}
		case ScopeType::SCOPE_LOCAL:
		{
			auto adjustedIdx = AdjustIdx(idx);
			auto local = CurrentFrame().BasePointer() + adjustedIdx;
			Push(_stack[local]);
			break;
		}
		case ScopeType::SCOPE_EXTERN:
		{
			auto adjustedIdx = AdjustIdx(idx);
			Push(_factory->New<BuiltInObj>(adjustedIdx));
			break;
		}
		case ScopeType::SCOPE_FREE:
		{
			auto adjustedIdx = AdjustIdx(idx);
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
		case ScopeType::SCOPE_GLOBAL:
		{
			auto adjustedIdx = AdjustIdx(idx);
			auto global = Pop();
			auto cloned = _factory->Clone(global);
			_globals[adjustedIdx] = cloned;
			break;
		}
		case ScopeType::SCOPE_LOCAL:
		{
			auto adjustedIdx = AdjustIdx(idx);
			auto local = Pop();
			auto localIdx = CurrentFrame().BasePointer() + adjustedIdx;
			auto cloned =  _factory->Clone(local);
			_stack[localIdx] = cloned;
			break;
		}
	}
}



