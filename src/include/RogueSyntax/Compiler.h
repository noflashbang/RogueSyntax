#pragma once
#include <StandardLib.h>
#include <AstNode.h>
#include <IObject.h>
#include <OpCode.h>

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
	CompilerError Compile(std::shared_ptr<INode> node);
	CompilerError Compile(INode* node);

	ByteCode GetByteCode() const;
	int AddConstant(std::shared_ptr<IObject> obj);
	int Emit(OpCode::Constants opcode, std::vector<int> operands);
	int AddInstruction(Instructions instructions);

	std::vector<std::string> GetErrors() const { return _errors; };
	inline bool HasErrors() const { return !_errors.empty(); };

	void NodeCompile(Program* program);
	void NodeCompile(BlockStatement* block);
	void NodeCompile(ExpressionStatement* expression);
	void NodeCompile(ReturnStatement* ret);
	void NodeCompile(LetStatement* let);
	void NodeCompile(Identifier* ident);
	void NodeCompile(IntegerLiteral* integer);
	void NodeCompile(BooleanLiteral* boolean);
	void NodeCompile(StringLiteral* string);
	void NodeCompile(DecimalLiteral* decimal);
	void NodeCompile(PrefixExpression* prefix);
	void NodeCompile(InfixExpression* infix);
	void NodeCompile(IfExpression* ifExpr);
	void NodeCompile(FunctionLiteral* function);
	void NodeCompile(CallExpression* call);
	void NodeCompile(ArrayLiteral* array);
	void NodeCompile(IndexExpression* index);
	void NodeCompile(HashLiteral* hash);
	void NodeCompile(NullLiteral* null);
	void NodeCompile(WhileExpression* whileExp);
	void NodeCompile(ForExpression* forExp);
	void NodeCompile(ContinueStatement* cont);
	void NodeCompile(BreakStatement* brk);

	static std::shared_ptr<Compiler> New();
private:
	Instructions _instructions;
	std::vector<std::shared_ptr<IObject>> _constants;

	std::vector<std::string> _errors;
	std::stack<CompilerErrorInfo> _errorStack;
};


