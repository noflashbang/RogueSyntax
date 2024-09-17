#pragma once
#include <StandardLib.h>
#include <AstNode.h>
#include <IObject.h>
#include <OpCode.h>


struct ByteCode
{
	Instructions Instructions;
	std::vector<std::shared_ptr<IObject>> Constants;
};

class Compiler
{
public:
	Compiler();
	~Compiler();
	void Compile(std::shared_ptr<INode> node);
	ByteCode GetByteCode() const;

	std::vector<std::string> GetErrors() const { return _errors; };

	static std::shared_ptr<Compiler> New();
private:
	Instructions _instructions;
	std::vector<std::shared_ptr<IObject>> _constants;

	std::vector<std::string> _errors;
};


