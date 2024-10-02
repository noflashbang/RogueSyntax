
#include "RogueSyntaxREPL.h"

void Repl::Start()
{
	std::string input;
	std::shared_ptr<Evaluator> eval = Evaluator::New(EvaluatorType::Stack);
	auto env = eval->MakeEnv();

	while (true)
	{
		std::string current;
		std::cout << _prompt;
		std::getline(std::cin, current);

		

		if (current.empty())
		{
			break;
		}
		else
		{
			input.append(current);
		}
	}

	Lexer lexer(input);
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

	auto result = eval->Eval(program, env);

	if (result == nullptr)
	{
		return;
	}

	if (result->IsThisA<ErrorObj>())
	{
		auto error = std::dynamic_pointer_cast<ErrorObj>(result);
		std::cout << "Error: " << error->Message << std::endl;

		//print out the source and error location
		auto location = error->Token.Location;
		std::cout << input << std::endl;
		for (size_t i = 1; i < location.Character; i++)
		{
			std::cout << " ";
		}
		std::cout << "^" << std::endl;
	}

	if (result != nullptr)
	{
		std::cout << result->Inspect() << std::endl;
	}

	eval->FreeEnv(env);
}

