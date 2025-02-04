
#include "RogueSyntaxCompiler.h"

void InteractiveCompiler::Run(const ByteCode& code)
{
	RogueSyntax syntax;
	auto vm = syntax.MakeVM(code);
	vm->Run();
	auto top = vm->LastPopped();
	
	if (top->IsThisA<ErrorObj>())
	{
		auto error = dynamic_cast<const ErrorObj*>(top);
		InternalPrintLine("Error: " + error->Message);		
	}
	else
	{
		_lastResult = top->Inspect();
		InternalPrintLine(_lastResult);
	}
}

std::string InteractiveCompiler::GetResult() const
{
	return _lastResult;
}

ByteCode InteractiveCompiler::Compile(const std::string& input)
{
	RogueSyntax syntax;
	auto compile = syntax.Compile(input, "RSUI");
	auto code = syntax.Link(compile);
	return code;
}

std::string InteractiveCompiler::Decompile(const ByteCode& input)
{
	RogueSyntax syntax;
	auto disassemble = syntax.Disassemble(input);
	return disassemble;
}

void InteractiveCompiler::InternalPrintLine(const std::string& line)
{
	_output.push_back(line);
}

void InteractiveCompiler::InternalPrintLocation(const std::string& line, const TokenLocation& location)
{
	_output.push_back(line + " at " + std::to_string(location.Line) + ":" + std::to_string(location.Column));
}


