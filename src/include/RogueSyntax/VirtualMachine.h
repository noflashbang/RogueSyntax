#pragma once
#include <StandardLib.h>
#include <OpCode.h>


#define STACK_SIZE 2048
#define GLOBAL_SIZE std::numeric_limits<int16_t>::max()
#define MAX_FRAMES 1024

struct Frame
{
public:
	Frame() : _ip(-1), _basePointer(0) {};
	Frame(std::shared_ptr<ClosureObj> fn, int basepointer) : _fn(fn), _basePointer(basepointer), _ip(0) {};
	Frame(std::shared_ptr<ClosureObj> fn, int ip, int basepointer) : _fn(fn), _ip(ip), _basePointer(basepointer) {};
	~Frame() {};
	inline const Instructions Instructions() const { return _fn->Function->FuncInstructions; };
	inline int Ip() const { return _ip; };
	inline void IncrementIp() { _ip++; };
	inline void IncrementIp(int amount) { _ip += amount; };
	inline void SetIp(int ip) { _ip = ip; };

	inline int BasePointer() const { return _basePointer; };
	inline void SetBasePointer(int bp) { _basePointer = bp; };
	inline ClosureObj* Closure() const { return _fn.get(); };
	inline std::shared_ptr<ClosureObj> ClosureRef() const { return _fn; };

private:
	int _ip;
	std::shared_ptr<ClosureObj> _fn;
	int _basePointer;
};


class RogueVM
{
public:
	RogueVM(const ByteCode& byteCode);
	RogueVM(const ByteCode& byteCode, std::shared_ptr<BuiltIn> externals);
	~RogueVM();

	void Run();

	//stack operations
	std::shared_ptr<IObject> Top() const;
	void Push(std::shared_ptr<IObject> obj);
	std::shared_ptr<IObject> Pop();
	std::shared_ptr<IObject> LastPoppped() const;

	//frame operations
	inline void IncrementFrameIp() { _frames[_frameIndex-1].IncrementIp(); };
	inline void IncrementFrameIp(int amount) { _frames[_frameIndex-1].IncrementIp(amount); };
	inline void SetFrameIp(int ip) { _frames[_frameIndex-1].SetIp(ip); };

	const Frame& CurrentFrame() const;
	void PushFrame(Frame frame);
	Frame PopFrame();

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

	void ExecutePrefix(OpCode::Constants opcode);
	void ExecuteIntegerPrefix(OpCode::Constants opcode, IntegerObj obj);
	void ExecuteDecimalPrefix(OpCode::Constants opcode, DecimalObj obj);
	void ExecuteBooleanPrefix(OpCode::Constants opcode, BooleanObj obj);
	void ExecuteNullPrefix(OpCode::Constants opcode, NullObj obj);

	void ExecuteIndexOperation(IObject* left, IObject* index);

	std::string MakeOpCodeError(const std::string& message, OpCode::Constants opcode);

private:
	std::array<Frame, MAX_FRAMES> _frames;
	int _frameIndex = 0;

	TypeCoercer _coercer;
	ByteCode _byteCode;
	std::array<std::shared_ptr<IObject>, STACK_SIZE> _stack;
	std::array<std::shared_ptr<IObject>, GLOBAL_SIZE> _globals;
	uint16_t _sp = 0;
	std::shared_ptr<BuiltIn> _externals;
};