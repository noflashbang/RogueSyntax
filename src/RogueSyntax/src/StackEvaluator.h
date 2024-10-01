#pragma once
#include <RogueSyntaxCore.h>
#include <variant>


class StackEvaluator : public Evaluator
{
public:
	std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, const uint32_t env) override;

	void NodeEval(Program* program) override;
	void NodeEval(BlockStatement* block) override;
	void NodeEval(ExpressionStatement* expression) override;
	void NodeEval(ReturnStatement* ret) override;
	void NodeEval(LetStatement* let) override;
	void NodeEval(Identifier* ident) override;
	void NodeEval(IntegerLiteral* integer) override;
	void NodeEval(BooleanLiteral* boolean) override;
	void NodeEval(StringLiteral* string) override;
	void NodeEval(DecimalLiteral* decimal) override;
	void NodeEval(PrefixExpression* prefix) override;
	void NodeEval(InfixExpression* infix) override;
	void NodeEval(IfStatement* ifExpr) override;
	void NodeEval(FunctionLiteral* function) override;
	void NodeEval(CallExpression* call) override;
	void NodeEval(ArrayLiteral* array) override;
	void NodeEval(IndexExpression* index) override;
	void NodeEval(HashLiteral* hash) override;
	void NodeEval(NullLiteral* null) override;
	void NodeEval(WhileStatement* whileExp) override;
	void NodeEval(ForStatement* forExp) override;
	void NodeEval(ContinueStatement* cont) override;
	void NodeEval(BreakStatement* brk) override;

	std::string Type() override { return "Stack"; }

private:

	int32_t _currentSignal;
	uint32_t _currentEnv;

	void Push_Eval(INode* node, const int32_t signal, const uint32_t env);
	void Pop_Eval();

	void Push_Result(std::shared_ptr<IObject> result);
	std::shared_ptr<IObject> Pop_Result();
	std::shared_ptr<IObject> Pop_ResultAndUnwrap();

	bool HasResults() const;
	bool ResultIsError() const;
	bool ResultIsReturn() const;
	bool ResultIsIdent() const;
	size_t ResultCount() const;

	std::stack<std::tuple<INode*, int32_t, uint32_t>> _stack;
	std::stack<std::shared_ptr<IObject>> _results;
};
