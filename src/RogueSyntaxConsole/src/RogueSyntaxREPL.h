#pragma once

#include <iostream>
#include <string>
#include <RogueSyntaxCore.h>
#include <RogueSyntax.h>

class Repl
{
public:

	void Start();

private:
	const std::string _prompt = ">> ";
	
};
