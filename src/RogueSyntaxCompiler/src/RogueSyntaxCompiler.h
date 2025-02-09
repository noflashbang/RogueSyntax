#pragma once

#include <iostream>
#include <string>
#include <RogueSyntaxCore.h>
#include <RogueSyntax.h>

class InteractiveCompiler
{
public:
	
	void Run(const ByteCode& code);
	std::vector<std::string> GetResult() const;

	ByteCode Compile(const std::string& input);
	std::string Decompile(const ByteCode& input);
	std::string GetPrompt() { return _prompt; };

	std::string Get_RTI_Error() const;
	std::string Get_RTI_Global(size_t idx);
	std::string Get_RTI_Local(size_t frame, size_t idx);


private:

	void ClearError() { _hasError = false; };
	void InternalOnError(const RogueVm_RuntimeError& error);
	void InternalOnBreak(const StackTrace& stack);

	void InternalPrintLine(const std::string& line);
	void InternalPrintLocation(const std::string& line, const TokenLocation& location);
	TokenLocation _lastLocation;
	std::string _lastResult;

	const std::string _prompt = ">>";
	std::string _input = "";	
	std::vector<std::string> _output;

	bool _hasError = false;
	RogueVm_RuntimeError _lastError;
};
