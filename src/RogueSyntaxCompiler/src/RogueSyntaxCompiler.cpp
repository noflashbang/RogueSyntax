
#include "RogueSyntaxCompiler.h"

void InteractiveCompiler::Run(const ByteCode& code)
{
	RogueSyntax syntax;
	_output.clear();
	_byteCode = code;

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
	_source = input;
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

StackTrace InteractiveCompiler::Get_RTI_StackTrace() const
{
	return _stackTrace;
}

std::string InteractiveCompiler::Get_RTI_Error() const
{
	if (!_hasError)
	{
		return "";
	}

	std::string scope = "LOCAL";
	auto debugInfo = _byteCode.DebugSymbols;
	auto error = _lastError.Message;
	auto stack = _stackTrace.ToString();

	std::string locals;
	auto frame = _stackTrace.Frames[_stackTrace.Frames.size() - 1];
	auto errorOffset = frame.FrameInstructionOffset;
	auto base = frame.BaseInstructionOffset;
	auto max = base;
	auto [op, operands, readOffset] = OpCode::ReadOperand(_byteCode.Instructions, base);
	if (op == OpCode::Constants::OP_LFUN)
	{
		max = base + operands[2];
	}
	else
	{
		//global frame
		scope = "GLOBAL";
		base = 0;
		max = _byteCode.Instructions.size();
	}

	auto idx = 0;
	for (auto& local : frame.Stack)
	{
		std::string symbol;
		auto symbolInRange = std::find_if(debugInfo.begin(), debugInfo.end(), [base, max, idx, scope](auto& ii) { return (base <= ii.Offset && ii.Offset <= max) && ii.Index == idx && ii.Scope == scope; }); 
		if (symbolInRange != debugInfo.end())
		{
			symbol = symbolInRange->Symbol;
		}
		
		std::string value = local.Value;
		if (ITypeTag::IsThisNameA<ClosureObj>(local.Type))
		{
			value = "fn{}";
		}
		locals += std::format("{}[{:0>2}]({: <16}) -> {}={}\n",scope, idx, local.Type, symbol, value);
		idx++;
	}

	std::string source = "";
	auto symbolAtError = std::find_if(debugInfo.begin(), debugInfo.end(), [errorOffset](auto& ii) { return errorOffset == ii.Offset; });
	if (symbolAtError != debugInfo.end())
	{
		auto line = symbolAtError->BaseToken.Location.Line;
		auto column = symbolAtError->BaseToken.Location.Character;

		std::string lineText;
		std::istringstream stream(_source);
		for (int i = 0; i < line; i++)
		{
			std::getline(stream, lineText);
		}

		source = std::format("Error @ LN:{:0>2} CH:{:0>2}\n{}\n{: <{}}^", line, column, lineText, "", column-1);
	}


	return error + '\n' + stack + '\n' + locals + '\n' + source;
}

std::string InteractiveCompiler::Get_RTI_Global(size_t idx)
{
	if (!_hasError || idx >= _stackTrace.Frames[0].Stack.size())
	{
		return "";
	}
	return std::format("{}:{}", _stackTrace.Frames[0].Stack[idx].Type, _stackTrace.Frames[0].Stack[idx].Value);
}

std::string InteractiveCompiler::Get_RTI_Local(size_t frame, size_t idx)
{
	if (!_hasError || frame >= _stackTrace.Frames.size() || idx >= _stackTrace.Frames[frame].Stack.size())
	{
		return "";
	}
	return std::format("{}:{}", _stackTrace.Frames[frame].Stack[idx].Type, _stackTrace.Frames[frame].Stack[idx].Value);
}

std::vector<DissaemblyDetail> InteractiveCompiler::Disassemble()
{
	return OpCode::Disassemble(_byteCode.Instructions);
}

std::vector<DebugSymbol> InteractiveCompiler::GetDebugSymbols()
{
	return _byteCode.DebugSymbols;
}

void InteractiveCompiler::InternalOnError(const RogueVm_RuntimeError& error)
{
	_hasError = true;
	_lastError = error;
	_stackTrace = error.StackTrace;
}

void InteractiveCompiler::InternalOnBreak(const StackTrace& stack)
{
	_stackTrace = stack;
}

void InteractiveCompiler::InternalPrintLine(const std::string& line)
{
	_output.push_back(line);
}

void InteractiveCompiler::InternalPrintLocation(const std::string& line, const TokenLocation& location)
{
	_output.push_back(line + " at " + std::to_string(location.Line) + ":" + std::to_string(location.Column));
}


