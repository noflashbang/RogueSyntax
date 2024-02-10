
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
		auto token = lexer.NextToken();
		while (token.Type != TokenType::TOKEN_EOF)
		{
			std::cout << "Type: " << token.Type.ToString() << " Literal: " << token.Literal << std::endl;
			token = lexer.NextToken();
		}
	}
}

