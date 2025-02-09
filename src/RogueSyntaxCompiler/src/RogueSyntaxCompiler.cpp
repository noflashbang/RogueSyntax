
#include "RogueSyntaxCompiler.h"

void InteractiveCompiler::Run(const ByteCode& code)
{
	RogueSyntax syntax;
	_output.clear();

	auto vm = syntax.MakeVM(code);

	vm->Set_RTI_ErrorCallback(std::bind(&InteractiveCompiler::InternalOnError, this, std::placeholders::_1));
	vm->Set_RTI_BreakCallback(std::bind(&InteractiveCompiler::InternalOnBreak, this, std::placeholders::_1));

	vm->Run();
	if (_hasError)
	{
		InternalPrintLine("Error: " + _lastError.Message);
	}
	else
	{
		auto top = vm->LastPopped();
		if (top->IsThisA<ErrorObj>())
		{
			auto error = dynamic_cast<const ErrorObj*>(top);
			InternalPrintLine("Error: " + error->Message);
		}
		else
		{
			InternalPrintLine(top->Inspect());
		}
	}
}

std::vector<std::string> InteractiveCompiler::GetResult() const
{
	return _output;
}

ByteCode InteractiveCompiler::Compile(const std::string& input)
{
	RogueSyntax syntax;
	auto compile = syntax.Compile(input, "");
	auto code = syntax.Link(compile);
	return code;
}

std::string InteractiveCompiler::Decompile(const ByteCode& input)
{
	RogueSyntax syntax;
	auto disassemble = syntax.Disassemble(input, true);
	return disassemble;
}

std::string InteractiveCompiler::Get_RTI_Error() const
{
	if (!_hasError)
	{
		return "";
	}
	return _lastError.ToString();
}

std::string InteractiveCompiler::Get_RTI_Global(size_t idx)
{
	if (!_hasError || idx >= _lastError.StackTrace.Frames[0].Stack.size())
	{
		return "";
	}
	return std::format("{}:{}", _lastError.StackTrace.Frames[0].Stack[idx].Type, _lastError.StackTrace.Frames[0].Stack[idx].Value);
}

std::string InteractiveCompiler::Get_RTI_Local(size_t frame, size_t idx)
{
	if (!_hasError || frame >= _lastError.StackTrace.Frames.size() || idx >= _lastError.StackTrace.Frames[frame].Stack.size())
	{
		return "";
	}
	return std::format("{}:{}", _lastError.StackTrace.Frames[frame].Stack[idx].Type, _lastError.StackTrace.Frames[frame].Stack[idx].Value);
}

void InteractiveCompiler::InternalOnError(const RogueVm_RuntimeError& error)
{
	_hasError = true;
	_lastError = error;
}

void InteractiveCompiler::InternalOnBreak(const StackTrace& stack)
{
	//noop
}

void InteractiveCompiler::InternalPrintLine(const std::string& line)
{
	_output.push_back(line);
}

void InteractiveCompiler::InternalPrintLocation(const std::string& line, const TokenLocation& location)
{
	_output.push_back(line + " at " + std::to_string(location.Line) + ":" + std::to_string(location.Column));
}


