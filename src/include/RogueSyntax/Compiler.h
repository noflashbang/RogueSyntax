#pragma once
#include <StandardLib.h>
#include <AstNode.h>
#include <IObject.h>
#include <OpCode.h>

#define SCOPE_GLOBAL "GLOBAL"
#define SCOPE_LOCAL "LOCAL"
#define SCOPE_EXTERN "EXTERN"
#define SCOPE_FREE "FREE"
#define SCOPE_FUNCTION "FUNCTION"



struct Symbol
{
	std::string Name;
	std::string Scope;
	int Index;
};

class SymbolTable
{
public:
	SymbolTable(std::shared_ptr<SymbolTable> outer);
	SymbolTable(std::string scope, std::shared_ptr<BuiltIn> externs);
	SymbolTable(std::shared_ptr<BuiltIn> externs);

	Symbol Define(const std::string& name);
	Symbol DefineFunctionName(const std::string& name);

	Symbol Resolve(const std::string& name);

	Symbol DefineFree(const Symbol& symbol);
	
	inline int NumberOfSymbols() const { return _store.size(); };
	inline std::shared_ptr<SymbolTable> Outer() const { return _outer; };

	std::vector<Symbol> FreeSymbols() { return _free; };

	static std::shared_ptr<SymbolTable> New(std::shared_ptr<BuiltIn> externs)
	{
		return std::make_shared<SymbolTable>(externs);
	}
	static std::shared_ptr<SymbolTable> New(std::string scope, std::shared_ptr<BuiltIn> externs)
	{
		return std::make_shared<SymbolTable>(scope, externs);
	}
	static std::shared_ptr<SymbolTable> New(std::shared_ptr<SymbolTable> outer)
	{
		return std::make_shared<SymbolTable>(outer);
	}
private:
	std::shared_ptr<BuiltIn> _externals;
	std::shared_ptr<SymbolTable> _outer;
	std::vector<Symbol> _store;
	uint32_t _nextIndex = 0;
	std::vector<Symbol> _free;
	std::string _scope;
};


enum LoopJumpType
{
	LOOP_JUMP_CONTINUE,
	LOOP_JUMP_BREAK,
};

struct LoopJump
{
	LoopJumpType Type;
	int Instruction;
};

struct CompilationUnit
{
	CompilationUnit(std::shared_ptr<SymbolTable> symbolTable)
	{
		SymbolTable = SymbolTable::New(symbolTable);
	}
	CompilationUnit(std::shared_ptr<BuiltIn> externs)
	{
		SymbolTable = SymbolTable::New(externs);
	}

	Instructions UnitInstructions;
	Instructions LastInstruction;
	Instructions PreviousLastInstruction;
	std::shared_ptr<SymbolTable> SymbolTable;

	std::stack<LoopJump> LoopJumps;

	void SetLastInstruction(const Instructions& instruction);
	int AddInstruction(Instructions instructions);

	void RemoveLastPop();
	bool LastInstructionIs(OpCode::Constants opcode);
	void RemoveLastInstruction();
	void ChangeOperand(int position, int operand);
	void ReplaceInstruction(int position, Instructions instructions);
};

enum class CompilerError
{
	NoError,
	UnknownOperator,
	UnknownError,
};

struct CompilerErrorInfo
{
	CompilerError Error;
	std::string Message;

	static CompilerErrorInfo New(CompilerError error, const std::string& message)
	{
		return CompilerErrorInfo{ error, message };
	}
};

struct ByteCode
{
	Instructions Instructions;
	std::vector<std::shared_ptr<IObject>> Constants;
};

class Compiler
{
public:
	Compiler();
	~Compiler();
	CompilerError Compile(std::shared_ptr<INode> node, const std::shared_ptr<BuiltIn>& externs);
	CompilerError Compile(INode* node);

	ByteCode GetByteCode() const;

	int EnterUnit();
	CompilationUnit ExitUnit();

	int AddConstant(std::shared_ptr<IObject> obj);
	int Emit(OpCode::Constants opcode, std::vector<int> operands);
	
	std::vector<std::string> GetErrors() const { return _errors; };
	inline bool HasErrors() const { return !_errors.empty(); };

	void NodeCompile(const Program* program);
	void NodeCompile(const BlockStatement* block);
	void NodeCompile(const ExpressionStatement* expression);
	void NodeCompile(const ReturnStatement* ret);
	void NodeCompile(const LetStatement* let);
	void NodeCompile(const Identifier* ident);
	void NodeCompile(const IntegerLiteral* integer);
	void NodeCompile(const BooleanLiteral* boolean);
	void NodeCompile(const StringLiteral* string);
	void NodeCompile(const DecimalLiteral* decimal);
	void NodeCompile(const PrefixExpression* prefix);
	void NodeCompile(const InfixExpression* infix);
	void NodeCompile(const IfStatement* ifExpr);
	void NodeCompile(const FunctionLiteral* function);
	void NodeCompile(const CallExpression* call);
	void NodeCompile(const ArrayLiteral* array);
	void NodeCompile(const IndexExpression* index);
	void NodeCompile(const HashLiteral* hash);
	void NodeCompile(const NullLiteral* null);
	void NodeCompile(const WhileStatement* whileExp);
	void NodeCompile(const ForStatement* forExp);
	void NodeCompile(const ContinueStatement* cont);
	void NodeCompile(const BreakStatement* brk);

	static std::shared_ptr<Compiler> New();
private:
	std::stack<CompilationUnit> _CompilationUnits;
	std::shared_ptr<BuiltIn> _externals;

	std::vector<std::shared_ptr<IObject>> _constants;
	
	std::vector<std::string> _errors;
	std::stack<CompilerErrorInfo> _errorStack;
};


