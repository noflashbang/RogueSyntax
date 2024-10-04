
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
	auto builtin = BuiltIn::New();
	Lexer lexer(_input);
	Parser parser(lexer);

	auto program = parser.ParseProgram();
	if (!parser.Errors().empty())
	{
		for (const auto& error : parser.Errors())
		{
			std::cout << "Parser error: " << error << std::endl;
		}
		return;
	}

	auto compile = Compiler::New();
	auto compError = compile->Compile(program, builtin);
	if (compile->HasErrors())
	{
		for (const auto& error : compile->GetErrors())
		{
			std::cout << "Compiler error: " << error << std::endl;
		}
		return;
	}

	auto bytecode = compile->GetByteCode();
	auto vm = RogueVM::New(bytecode, builtin);

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
	auto builtin = BuiltIn::New();
	Lexer lexer(_input);
	Parser parser(lexer);

	auto program = parser.ParseProgram();
	if (!parser.Errors().empty())
	{
		for (const auto& error : parser.Errors())
		{
			std::cout << "Parser error: " << error << std::endl;
		}
		return;
	}

	auto compile = Compiler::New();
	auto compError = compile->Compile(program, builtin);
	if (compile->HasErrors())
	{
		for (const auto& error : compile->GetErrors())
		{
			std::cout << "Compiler error: " << error << std::endl;
		}
		return;
	}

	auto bytecode = compile->GetByteCode();
	std::cout << OpCode::PrintInstructions(bytecode.Instructions) << std::endl;
}

