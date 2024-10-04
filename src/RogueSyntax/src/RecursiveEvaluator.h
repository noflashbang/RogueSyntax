#pragma once
#include <RogueSyntaxCore.h>


class RecursiveEvaluator : public Evaluator
{
public:
	const IObject* Eval(const INode* node, const uint32_t env) override;
	const IObject* Eval(const INode* node);

	void NodeEval(const Program* program) override;
	void NodeEval(const BlockStatement* block) override;
	void NodeEval(const ExpressionStatement* expression) override;
	void NodeEval(const ReturnStatement* ret) override;
	void NodeEval(const LetStatement* let) override;
	void NodeEval(const Identifier* ident) override;
	void NodeEval(const IntegerLiteral* integer) override;
	void NodeEval(const BooleanLiteral* boolean) override;
	void NodeEval(const StringLiteral* string) override;
	void NodeEval(const DecimalLiteral* decimal) override;
	void NodeEval(const PrefixExpression* prefix) override;
	void NodeEval(const InfixExpression* infix) override;
	void NodeEval(const IfStatement* ifExpr) override;
	void NodeEval(const FunctionLiteral* function) override;
	void NodeEval(const CallExpression* call) override;
	void NodeEval(const ArrayLiteral* array) override;
	void NodeEval(const IndexExpression* index) override;
	void NodeEval(const HashLiteral* hash) override;
	void NodeEval(const NullLiteral* null) override;
	void NodeEval(const WhileStatement* whileExp) override;
	void NodeEval(const ForStatement* forExp) override;
	void NodeEval(const ContinueStatement* cont) override;
	void NodeEval(const BreakStatement* brk) override;

	std::string Type() override { return "Recursive"; }

private:
	std::vector<const IObject*> EvalExpressions(const std::vector<IExpression*>& expressions);
	const IObject* ApplyFunction(const FunctionObj* func, const std::vector<const IObject*>& args);

	std::stack<const IObject*> _results;
	uint32_t _env;
};