#include "RogueSyntax.h"
#include "pch.h"
#include "RogueSyntax.h"
#include "Compiler.h"
#include "Linker.h"

RogueSyntax::RogueSyntax() : _objectStore(std::make_shared<ObjectStore>()), _builtIn(std::make_shared<BuiltIn>(_objectStore->Factory()))
{
}

RogueSyntax::~RogueSyntax()
{
}

std::shared_ptr<Program> RogueSyntax::Parse(const std::string& input, const std::string& unit) const
{
	Lexer lexer(input);
	Parser parser(lexer);
	return parser.ParseProgram(unit);
}

ObjectCode RogueSyntax::Compile(const std::string& input, const std::string& unit) const
{
	Lexer lexer(input);
	Parser parser(lexer);
	auto program = parser.ParseProgram(unit);
	if (!parser.Errors().empty())
	{
		throw std::runtime_error("Parser error"); //TODO: better error handling
	}

	Compiler compiler(_objectStore->Factory());
	return compiler.Compile(program, _builtIn, "PRG");
}

std::string RogueSyntax::Disassemble(const ByteCode& code, bool includeDebugSymbols) const
{
	if (includeDebugSymbols)
	{
		return OpCode::PrintInstructionsWithDebug(code);
	}
	return OpCode::PrintInstructions(code.Instructions);
}

ByteCode RogueSyntax::Link(const ObjectCode& objectCode) const
{
	Linker linker;
	return linker.Link(objectCode);
}

std::shared_ptr<Evaluator> RogueSyntax::MakeEvaluator(EvaluatorType type) const
{
	return Evaluator::New(type, _objectStore->Factory());
}

std::shared_ptr<RogueVM> RogueSyntax::MakeVM(ByteCode code) const
{
	return std::make_shared<RogueVM>(code, _builtIn, _objectStore->Factory());
}

const IObject* RogueSyntax::QuickEval(EvaluatorType type, const std::string& input) const
{
	auto program = Parse(input, "QUICKEVAL");
	if (program == nullptr)
	{
		return nullptr;
	}
	auto eval = MakeEvaluator(type);
	try
	{
		return eval->Eval(program, _builtIn);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}

void RogueSyntax::RegisterBuiltIn(const std::string& name, std::function<IObject* (const ObjectFactory* factory, const std::vector<const IObject*>& args)> func)
{
	_builtIn->RegisterBuiltIn(name, func);
}