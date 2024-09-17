#include <pch.h>

Compiler::Compiler()
{
}

Compiler::~Compiler()
{
}

void Compiler::Compile(std::shared_ptr<INode> node)
{
}

ByteCode Compiler::GetByteCode() const
{
	return ByteCode {_instructions, _constants };
}

std::shared_ptr<Compiler> Compiler::New()
{
	return std::make_shared<Compiler>();
}

