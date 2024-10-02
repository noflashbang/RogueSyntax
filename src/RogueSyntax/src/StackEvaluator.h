#pragma once
#include <RogueSyntaxCore.h>
#include <variant>


class StackEvaluator : public Evaluator
{
public:
	std::shared_ptr<IObject> Eval(const INode* node, const uint32_t env) override;

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

	std::string Type() override { return "Stack"; }

private:

	int32_t _currentSignal;
	uint32_t _currentEnv;

	void Push_Eval(const INode* node, const int32_t signal, const uint32_t env);
	void Pop_Eval();

	void Push_Result(std::shared_ptr<IObject> result);
	std::shared_ptr<IObject> Pop_Result();
	std::shared_ptr<IObject> Pop_ResultAndUnwrap();

	bool HasResults() const;
	bool ResultIsError() const;
	bool ResultIsReturn() const;
	bool ResultIsIdent() const;
	size_t ResultCount() const;

	std::stack<std::tuple<const INode*, int32_t, uint32_t>> _stack;
	std::stack<std::shared_ptr<IObject>> _results;
};
