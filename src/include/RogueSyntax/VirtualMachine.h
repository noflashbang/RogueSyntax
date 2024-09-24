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
	std::shared_ptr<IObject> GetTop() const { return _sp > 0 ? _stack[_sp-1] : NullObj::NULL_OBJ_REF; }
	void Push(std::shared_ptr<IObject> obj) { if (_sp >= _stack.size()) { throw std::exception("Stack Overflow"); }  _stack[_sp++] = obj; }
	std::shared_ptr<IObject> Pop() { if (_sp == 0) { throw std::exception("Stack Underflow"); } return _stack[--_sp]; }

private:
	ByteCode _byteCode;
	std::array<std::shared_ptr<IObject>, STACK_SIZE> _stack;
	uint16_t _sp = 0;
	
};