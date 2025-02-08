#pragma once
#include <StandardLib.h>
#include <AstNode.h>
#include <IObject.h>
#include <OpCode.h>
#include "CompilationUnit.h"
#include "SymbolTable.h"

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

class Compiler
{
public:
	Compiler(const std::shared_ptr<ObjectFactory> factory);
	~Compiler();
	ObjectCode Compile(const std::shared_ptr<Program>& program, const std::shared_ptr<BuiltIn>& externs, const std::string& unitName);
	inline bool HasErrors() const { return !_errors.empty(); };
	std::vector<std::string> GetErrors() const { return _errors; };

	void NodeCompile(const Program* program, const std::string& unitName);
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

	void EnterScope(const std::string& scope);
	void ExitScope();

	uint32_t AddConstant(const IObject* obj);
	int Emit(OpCode::Constants opcode, std::vector<uint32_t> operands);
	int Emit(OpCode::Constants opcode, std::vector<uint32_t> operands, Instructions data);
	int EmitGet(Symbol symbol);
	int EmitSet(Symbol symbol);

	void EmitDebugSymbol(const  INode* node);

private:
	SymbolTable _symbolTable;
	std::stack<CompilationUnit> _CompilationUnits;

	std::shared_ptr<BuiltIn> _externals;
	std::vector<const IObject*> _constants;

	std::vector<std::string> _errors;
	std::stack<CompilerErrorInfo> _errorStack;
	std::shared_ptr<ObjectFactory> _factory;
};


