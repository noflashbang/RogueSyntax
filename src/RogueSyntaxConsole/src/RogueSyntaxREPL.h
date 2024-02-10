#pragma once

#include <iostream>
#include <string>
#include <RogueSyntaxCore.h>

class Repl
{
public:

	void Start();

private:
	const std::string _prompt = ">> ";
	
};
