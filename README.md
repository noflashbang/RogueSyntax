# RogueSyntax

*~~ Rogue//Syntax ~~* is a hobby project used to deepen the understanding of various computer science areas including:
- Lexer
- Parser
- AST
- Interpreter
- Compiler
- Virtual Machine
- IDE (debugging)

For a simple toy language.

## Project Highlights

- **Lexer**: Tokenizes the input source code into meaningful symbols.
- **Parser**: Analyzes the tokenized input to generate a syntax tree.
- **Interpreter**: Executes the syntax tree directly.
- **Compiler**: Translates the syntax tree into machine code or bytecode.
- **Virtual Machine**: Executes the compiled bytecode.
- **IDE**: Provides debugging capabilities for the language, Virtual Machine and Compiler.

## Getting Started

Project is built using CMake and Ninja. Using Visual Studio or VSCode is recommended.

The "Install" targets are the intended target for building and running the projects. This is especially important for the IDE (RogueSyntaxCompiler) as it requires the resources to be copied to the output directory.

Sample programs for the language are located in the `example` directory of the RogueSyntaxCompiler project.

## Overview

The project is divided into the following sub-projects:

- **RogueSyntax** - Contains the implementation of the lexer/parser/interpreter/compiler/vm.
- **RogueSyntaxConsole** - Simple REPL (Read-Eval-Print Loop) for the language.
- **RogueSyntaxCompiler** - Contains the IDE implemenation.
- **RogueSyntaxTest** - Contains all the tests for the lexer/parser/interpreter/compiler/vm.

### Prerequisites

- C++20 compatible compiler
- CMake
- Ninja

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [Building an Interpreter in Go](https://interpreterbook.com/) - Thorsten Ball's incredible book on building an interpreter.
- [Building a Compiler in Go](https://compilerbook.com/) - Thorsten Ball's incredible book on building a compiler.
- [Catch2](https://github.com/catchorg/Catch2) - Unit Test Framework for C++
- [clay](https://github.com/nicbarker/clay) - UI Layout Library
- [CMake](https://cmake.org/) - Build system
- [Ninja](https://ninja-build.org/) - Build system


