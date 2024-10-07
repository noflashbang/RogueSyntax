#pragma once
#include <StandardLib.h>
#include <AstNode.h>
#include <IObject.h>
#include <OpCode.h>

enum class ScopeType : uint8_t
{
	SCOPE_GLOBAL,
	SCOPE_LOCAL,
	SCOPE_EXTERN,
	SCOPE_FREE,
	SCOPE_FUNCTION,
};

struct Symbol
{
	ScopeType Type;
	std::string Name;
	std::string MangledName;
	int Index;

	uint32_t EncodedIdx();
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
	CompilationUnit(ScopeType type, const std::string& context)
	{
		_type = type;
		_context = context;
	}

	CompilationUnit(ScopeType type, const std::string& context)
	{
		_type = type;
		_context = context;
	}

	Instructions UnitInstructions;
	Instructions LastInstruction;
	Instructions PreviousLastInstruction;

	std::stack<LoopJump> LoopJumps;

	void SetLastInstruction(const Instructions& instruction);
	int AddInstruction(Instructions instructions);

	void RemoveLastPop();
	bool LastInstructionIs(OpCode::Constants opcode);
	void RemoveLastInstruction();
	void ChangeOperand(int position, uint32_t operand);
	void ReplaceInstruction(int position, Instructions instructions);

	const std::string context() const { return _context; };

	Symbol Define(const std::string& name);
	Symbol DefineFunctionName(const std::string& name);
	Symbol DefineExternal(const std::string& name, int idx);
	Symbol Resolve(const std::string& name);
	Symbol DefineFree(const Symbol& symbol);

	inline int NumberOfSymbols() const { return _store.size(); };


private:
	ScopeType _type;
	std::string _context;
	std::vector<Symbol> _store;
	uint32_t _nextSymIndex = 0;
	uint32_t _nextFreeIdx = 0;
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
	std::vector<const IObject*> Constants;
};

class Compiler
{
public:
	Compiler(const std::shared_ptr<ObjectFactory> factory);
	~Compiler();
	ByteCode Compile(const std::shared_ptr<Program>& program, const std::shared_ptr<BuiltIn>& externs);
	inline bool HasErrors() const { return !_errors.empty(); };
	std::vector<std::string> GetErrors() const { return _errors; };

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

protected:

	CompilerError Compile(INode* node);
	int EnterUnit(const std::string& context);
	CompilationUnit ExitUnit();

	uint32_t AddConstant(const IObject* obj);
	int Emit(OpCode::Constants opcode, std::vector<uint32_t> operands);
	int EmitGet(Symbol symbol);
	int EmitSet(Symbol symbol);

private:
	std::stack<CompilationUnit> _CompilationUnits;
	std::shared_ptr<BuiltIn> _externals;
	std::vector<const IObject*> _constants;
	
	std::vector<std::string> _errors;
	std::stack<CompilerErrorInfo> _errorStack;
	std::shared_ptr<ObjectFactory> _factory;
};


