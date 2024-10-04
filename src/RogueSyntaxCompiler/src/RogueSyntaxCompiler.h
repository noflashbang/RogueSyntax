#pragma once

#include <iostream>
#include <string>
#include <RogueSyntaxCore.h>
#include <RogueSyntax.h>

class InteractiveCompiler
{
public:

	void Start();
	void Run();
	void PrintDecompile();

private:
	const std::string _prompt = ">> ";
	std::string _input = "";	
};
