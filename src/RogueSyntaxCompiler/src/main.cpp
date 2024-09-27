
#include <RogueSyntaxCore.h>
#include "RogueSyntaxCompiler.h"

int main(int argc, char *argv[])
{
	InteractiveCompiler console;
	console.Start();
	console.PrintDecompile();
	console.Run();
	return 0;
}

