#pragma once

#include <iostream>
#include <string>
#include <RogueSyntaxCore.h>

class InteractiveCompiler
{
public:

	void Start();
	void Run();

private:
	const std::string _prompt = ">> ";
	std::string _input = "";	
};
