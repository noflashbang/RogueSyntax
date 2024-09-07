
#include "RogueSyntaxREPL.h"

void Repl::Start()
{
	std::string input;
	auto env = Environment::New();

	while (true)
	{
		std::cout << _prompt;
		std::getline(std::cin, input);

		if(input == "")
		{
			break;
		}

		Lexer lexer(input);
		Parser parser(lexer);

		auto program = parser.ParseProgram();
		if (parser.Errors().size() > 0)
		{
			for (const auto& error : parser.Errors())
			{
				std::cout << "Parser error: " << error << std::endl;
			}
			continue;
		}

		std::shared_ptr<Evaluator> eval = std::make_shared<StackEvaluator>();
		auto result = eval->Eval(program, env);

		if (result == nullptr)
		{
			continue;
		}

		if (result->Type() == ObjectType::ERROR_OBJ)
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
	}
}

