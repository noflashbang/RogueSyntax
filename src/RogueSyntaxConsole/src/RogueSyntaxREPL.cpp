
#include "RogueSyntaxREPL.h"

void Repl::Start()
{
	RogueSyntax syntax;
	std::string input;

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

	auto result = syntax.QuickEval(EvaluatorType::Stack, input);

	if (result == nullptr)
	{
		return;
	}

	if (result->IsThisA<ErrorObj>())
	{
		auto error = dynamic_cast<const ErrorObj*>(result);
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

