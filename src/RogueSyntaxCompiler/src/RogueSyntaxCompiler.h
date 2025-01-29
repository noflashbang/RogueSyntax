#pragma once

#include <iostream>
#include <string>
#include <RogueSyntaxCore.h>
#include <RogueSyntax.h>

class InteractiveCompiler
{
public:
	
	void Run(const ByteCode& code);
	std::string GetResult() const;

	ByteCode Compile(const std::string& input);
	std::string Decompile(const ByteCode& input);


	
private:

	void InternalPrintLine(const std::string& line);
	void InternalPrintLocation(const std::string& line, const TokenLocation& location);
	TokenLocation _lastLocation;
	std::string _lastResult;

	const std::string _prompt = ">>";
	std::string _input = "";	
	std::vector<std::string> _output;
};
