
#include "RogueSyntaxREPL.h"

void Repl::Start()
{
	std::string input;
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

		std::cout << program.ToString() << std::endl;


	}
}

