#include "Linker.h"
#include <pch.h>


Linker::Linker()
{
}

Linker::~Linker()
{
}

ByteCode Linker::Link(const ObjectCode& objectCode)
{
	return Link(std::vector<ObjectCode>{ objectCode });
}

ByteCode Linker::Link(const std::vector<ObjectCode>& objectCodes)
{
	ByteCode code;
	
	for (auto& obj : objectCodes)
	{
		LinkConstants(code, obj);
		LinkInstructions(code, obj);
	}

	return code;
}

void Linker::LinkConstants(ByteCode& code, const ObjectCode& objectCode)
{

	for (auto& constant : objectCode.Constants)
	{
		std::string sym = "CONST_" + constant->Id();

		
		

	}
}

void Linker::LinkInstructions(ByteCode& code, const ObjectCode& objectCode)
{
	code.Instructions.reserve(objectCode.Instructions.size());
	code.Instructions = objectCode.Instructions;
}



