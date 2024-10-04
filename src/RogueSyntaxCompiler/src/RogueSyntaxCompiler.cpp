
#include "RogueSyntaxCompiler.h"

void InteractiveCompiler::Start()
{
	std::string input;
	while (true)
	{
		std::cout << _prompt;
		std::getline(std::cin, input);

		if(input.empty())
		{
			break;
		}
		else
		{
			_input.append(input);
		}
	}
}

void InteractiveCompiler::Run()
{
	RogueSyntax syntax;
	auto vm = syntax.MakeVM(syntax.Compile(_input));
	vm->Run();
	//auto top = vm->LastPoppped();
	auto top = vm->Top();

	if (top->IsThisA<ErrorObj>())
	{
		auto error = dynamic_cast<const ErrorObj*>(top);
		std::cout << "Error: " << error->Message << std::endl;

		//print out the source and error location
		auto location = error->Token.Location;
		std::cout << _input << std::endl;
		for (size_t i = 1; i < location.Character; i++)
		{
			std::cout << " ";
		}
		std::cout << "^" << std::endl;
	}

	//if (top != nullptr)
	//{
	//	std::cout << top->Inspect() << std::endl;
	//}
}

void InteractiveCompiler::PrintDecompile()
{
	RogueSyntax syntax;
	auto compile = syntax.Compile(_input);;
	std::cout << OpCode::PrintInstructions(compile.Instructions) << std::endl;
}

