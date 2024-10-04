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
	Frame(const ClosureObj* fn, int basepointer) : _fn(fn), _basePointer(basepointer), _ip(0) {};
	Frame(const ClosureObj* fn, int ip, int basepointer) : _fn(fn), _ip(ip), _basePointer(basepointer) {};
	~Frame() {};
	inline const Instructions Instructions() const { return _fn->Function->FuncInstructions; };
	inline int Ip() const { return _ip; };
	inline void IncrementIp() { _ip++; };
	inline void IncrementIp(int amount) { _ip += amount; };
	inline void SetIp(int ip) { _ip = ip; };

	inline int BasePointer() const { return _basePointer; };
	inline void SetBasePointer(int bp) { _basePointer = bp; };
	inline const ClosureObj* Closure() const { return _fn; };
	inline const ClosureObj* ClosureRef() const { return _fn; };

private:
	int _ip;
	const ClosureObj* _fn;
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
	const IObject* Top() const;
	void Push(const IObject* obj);
	const IObject* Pop();
	const IObject* LastPoppped() const;

	//frame operations
	inline void IncrementFrameIp() { _frames[_frameIndex-1].IncrementIp(); };
	inline void IncrementFrameIp(int amount) { _frames[_frameIndex-1].IncrementIp(amount); };
	inline void SetFrameIp(int ip) { _frames[_frameIndex-1].SetIp(ip); };

	const Frame& CurrentFrame() const;
	void PushFrame(Frame frame);
	Frame PopFrame();

	static std::shared_ptr<RogueVM> New(const ByteCode& byteCode, std::shared_ptr<BuiltIn> externals) { return std::make_shared<RogueVM>(byteCode, externals); }

protected:
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
	GetSetType GetTypeFromIdx(int idx);

private:
	ObjectStore _store;
	std::array<Frame, MAX_FRAMES> _frames;
	int _frameIndex = 0;

	TypeCoercer _coercer;
	ByteCode _byteCode;
	std::array<const IObject*, STACK_SIZE> _stack;
	std::array<const IObject*, GLOBAL_SIZE> _globals;
	uint16_t _sp = 0;
	std::shared_ptr<BuiltIn> _externals;
};