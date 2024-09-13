#pragma once
#include <RogueSyntaxCore.h>

class StackEvaluator : public Evaluator
{
public:
	std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, const std::shared_ptr<Environment>& env, const std::shared_ptr<BuiltIn>& builtIn) override;
private:

	void Push_Eval(const INode* node, int32_t signal, const Environment* env);
	void Pop_Eval();

	void Push_Result(std::shared_ptr<IObject> result);
	std::shared_ptr<IObject> Pop_Result();
	std::shared_ptr<IObject> Pop_ResultAndUnwrap();

	bool HasResults() const;
	bool ResultIsError() const;
	bool ResultIsReturn() const;
	bool ResultIsIdent() const;

	void Handler_Program(const Program* program, int32_t signal, const Environment* env);
	void Handler_BlockStatement(const BlockStatement* block, int32_t signal, const Environment* env);
	void Handler_ExpressionStatement(const ExpressionStatement* expression, int32_t signal, const Environment* env);
	void Handler_ReturnStatement(const ReturnStatement* ret, int32_t signal, const Environment* env);
	void Handler_LetStatement(const LetStatement* let, int32_t signal, const Environment* env);
	void Handler_Identifier(const Identifier* ident, int32_t signal, const Environment* env);
	void Handler_IntegerLiteral(const IntegerLiteral* integer, int32_t signal, const Environment* env);
	void Handler_BooleanLiteral(const BooleanLiteral* boolean, int32_t signal, const Environment* env);
	void Handler_StringLiteral(const StringLiteral* string, int32_t signal, const Environment* env);
	void Handler_DecimalLiteral(const DecimalLiteral* decimal, int32_t signal, const Environment* env);
	void Handler_PrefixExpression(const PrefixExpression* prefix, int32_t signal, const Environment* env);
	void Handler_InfixExpression(const InfixExpression* infix, int32_t signal, const Environment* env);
	void Handler_IfExpression(const IfExpression* ifExpr, int32_t signal, const Environment* env);
	void Handler_FunctionLiteral(const FunctionLiteral* function, int32_t signal, const Environment* env);
	void Handler_CallExpression(const CallExpression* call, int32_t signal, const Environment* env);
	void Handler_ArrayLiteral(const ArrayLiteral* array, int32_t signal, const Environment* env);
	void Handler_IndexExpression(const IndexExpression* index, int32_t signal, const Environment* env);
	void Handler_HashLiteral(const HashLiteral* hash, int32_t signal, const Environment* env);
	void Handler_NullLiteral(const NullLiteral* null, int32_t signal, const Environment* env);
	void Handler_WhileExpression(const WhileExpression* whileExp, int32_t signal, const Environment* env);
	void Handler_ForStatement(const ForExpression* forExp, int32_t signal, const Environment* env);
	void Handler_ContinueStatement(const ContinueStatement* cont, int32_t signal, const Environment* env);
	void Handler_BreakStatement(const BreakStatement* brk, int32_t signal, const Environment* env);

	std::stack<std::tuple<const INode*, int32_t, const Environment*>> _stack;
	std::stack<std::shared_ptr<IObject>> _results;
};