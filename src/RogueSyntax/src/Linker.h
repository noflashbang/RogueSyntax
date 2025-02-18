#pragma once
#include <StandardLib.h>
#include <AstNode.h>
#include <IObject.h>
#include <OpCode.h>
#include "CompilationUnit.h"
#include "SymbolTable.h"


class Linker
{
public:
	Linker();
	~Linker();

	ByteCode Link(const ObjectCode& objectCode);
	ByteCode Link(const std::vector<ObjectCode>& objectCodes);

private:
	void LinkInstructions(ByteCode& code, const ObjectCode& objectCode);

	std::unordered_map<std::string, uint32_t> _symbolOffsets;
};