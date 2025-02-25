#pragma once
#include <StandardLib.h>
#include <OpCode.h>

#define STACK_SIZE 2048
#define GLOBAL_SIZE std::numeric_limits<int16_t>::max()
#define MAX_FRAMES 1024

struct StackValue
{
	std::string Type;
	std::string Value;
};

struct FrameTrace
{
	size_t FrameIdx;
	size_t AbsoluteInstructionOffest;
	size_t BaseInstructionOffset;
	size_t FrameInstructionOffset;

	std::vector<StackValue> Stack;
};

struct StackTrace
{
	std::vector<FrameTrace> Frames;
	std::string ToString() const;
};

struct RogueVm_RuntimeError
{
	std::string Message;
	StackTrace StackTrace;

	std::string ToString() const;
};

struct Frame
{
public:
	Frame() : _ip(-1), _basePointer(0) {};
	Frame(const ClosureObj* fn, int basepointer) : _fn(fn), _basePointer(basepointer), _ip(0), _beforeIp(0) {};
	Frame(const ClosureObj* fn, int ip, int basepointer) : _fn(fn), _ip(ip), _beforeIp(0), _basePointer(basepointer) {};
	~Frame() {};
	inline const RSInstructions& Instructions() const { return _fn->Function->FuncInstructions; };
	inline int Ip() const { return _ip; };
	inline int BaseOffset() const { return _fn->Function->FuncOffset; };
	inline void IncrementIp() { _ip++; };
	inline void IncrementIp(int amount) { _ip += amount; };
	inline void SetIp(int ip) { _ip = ip; };
	
	inline void SaveBeforeIp() const { _beforeIp = _ip; };
	inline int BeforeIp() const { return _beforeIp; };

	inline int BasePointer() const { return _basePointer; };
	inline void SetBasePointer(int bp) { _basePointer = bp; };
	inline const ClosureObj* Closure() const { return _fn; };
	inline const ClosureObj* ClosureRef() const { return _fn; };

private:
	mutable int _beforeIp;
	int _ip;
	const ClosureObj* _fn;
	int _basePointer;
};


class RogueVM
{
public:
	RogueVM(const ByteCode& byteCode, const std::shared_ptr<ObjectFactory>& factory);
	RogueVM(const ByteCode& byteCode, const std::shared_ptr<BuiltIn>& externals, const std::shared_ptr<ObjectFactory>& factory);
	~RogueVM();

	void Run();
	void Set_RTI_ErrorCallback(const std::function<void(const RogueVm_RuntimeError&)>& onError);
	void Set_RTI_BreakCallback(const std::function<void(const StackTrace&)>& onBreak);

	const IObject* Top() const;
	const IObject* LastPopped() const;
	const Frame& CurrentFrame() const;

protected:

	void OnErrorInternal(const RogueVm_RuntimeError& error);
	void OnBreakInternal(const StackTrace& stack);

	FrameTrace GetFrameTrace(const size_t idx, const Frame& frame) const;
	StackTrace GetRuntimeInfo() const;
	std::string PrintStack() const;

	void Execute();

	//stack operations
	void Push(const IObject* obj);
	const IObject* Pop();

	//frame operations
	inline void IncrementFrameIp() { _frames[_frameIndex - 1].IncrementIp(); };
	inline void IncrementFrameIp(int amount) { _frames[_frameIndex - 1].IncrementIp(amount); };
	inline void SetFrameIp(int ip) { _frames[_frameIndex - 1].SetIp(ip); };

	void PushFrame(Frame frame);
	Frame PopFrame();

	template<std::integral T>
	T ReadOperand(uint8_t width)
	{
		T operand = 0;
		int currentOffset = CurrentFrame().Ip();
		const auto& instructions = CurrentFrame().Instructions();
		//use switch case to handle different operand widths - fallthrough is intentional
		switch (width)
		{
		case 4:
			operand = instructions[currentOffset] << 24;
			currentOffset++;
		case 3:
			operand |= instructions[currentOffset] << 16;
			currentOffset++;
		case 2:
			operand |= instructions[currentOffset] << 8;
			currentOffset++;
		case 1:
			operand |= instructions[currentOffset];
			currentOffset++;
		}
		IncrementFrameIp(width);
		return operand;
	}

	void ExecuteArithmeticInfix(OpCode::Constants opcode);
	void ExecuteIntegerArithmeticInfix(OpCode::Constants opcode, const IntegerObj* left, const IntegerObj* right);
	void ExecuteDecimalArithmeticInfix(OpCode::Constants opcode, const DecimalObj* left, const DecimalObj* right);
	void ExecuteStringArithmeticInfix(OpCode::Constants opcode, const StringObj* left, const StringObj* right);
	
	void ExecuteComparisonInfix(OpCode::Constants opcode);
	void ExecuteIntegerComparisonInfix(OpCode::Constants opcode, const IntegerObj* left, const IntegerObj* right);
	void ExecuteDecimalComparisonInfix(OpCode::Constants opcode, const DecimalObj* left, const DecimalObj* right);
	void ExecuteStringComparisonInfix(OpCode::Constants opcode, const StringObj* left, const StringObj* right);
	void ExecuteBooleanComparisonInfix(OpCode::Constants opcode, const BooleanObj* left, const BooleanObj* right);
	void ExecuteNullComparisonInfix(OpCode::Constants opcode, const NullObj* left, const NullObj* right);

	void ExecutePrefix(OpCode::Constants opcode);
	void ExecuteIntegerPrefix(OpCode::Constants opcode, const IntegerObj* obj);
	void ExecuteDecimalPrefix(OpCode::Constants opcode, const DecimalObj* obj);
	void ExecuteBooleanPrefix(OpCode::Constants opcode, const BooleanObj* obj);
	void ExecuteNullPrefix(OpCode::Constants opcode, const NullObj* obj);

	void ExecuteIndexOperation(const IObject* left, const IObject* index);

	std::string MakeOpCodeError(const std::string& message, OpCode::Constants opcode);

	void ExecuteGetInstruction(int idx);
	void ExecuteSetInstruction(int idx);

private:

	std::function<void(const RogueVm_RuntimeError&)> _onError;
	std::function<void(const StackTrace&)> _onBreak;

	int _frameIndex = 0;
	uint16_t _sp = 0;
	std::shared_ptr<ObjectFactory> _factory;
	std::shared_ptr<BuiltIn> _externals;
	TypeCoercer _coercer;
	std::array<const IObject*, STACK_SIZE> _stack{0};
	std::array<const IObject*, GLOBAL_SIZE> _globals{0};
	const IObject* _outputRegister = nullptr;
	std::array<Frame, MAX_FRAMES> _frames;
	ByteCode _byteCode;
};