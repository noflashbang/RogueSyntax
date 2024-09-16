#pragma once
#include <RogueSyntaxCore.h>


class RecursiveEvaluator : public Evaluator
{
public:
	std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, const uint32_t env) override;
	std::shared_ptr<IObject> Eval(INode* node, const uint32_t env);

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
	void NodeEval(IfExpression* ifExpr) override;
	void NodeEval(FunctionLiteral* function) override;
	void NodeEval(CallExpression* call) override;
	void NodeEval(ArrayLiteral* array) override;
	void NodeEval(IndexExpression* index) override;
	void NodeEval(HashLiteral* hash) override;
	void NodeEval(NullLiteral* null) override;
	void NodeEval(WhileExpression* whileExp) override;
	void NodeEval(ForExpression* forExp) override;
	void NodeEval(ContinueStatement* cont) override;
	void NodeEval(BreakStatement* brk) override;

	std::string Type() override { return "Recursive"; }

private:
	std::vector<std::shared_ptr<IObject>> EvalExpressions(const std::vector<std::shared_ptr<IExpression>>& expressions, const uint32_t env);
	std::shared_ptr<IObject> ApplyFunction(const uint32_t env, const std::shared_ptr<FunctionObj>& func, const std::vector<std::shared_ptr<IObject>>& args);

	std::stack<std::shared_ptr<IObject>> _results;
	uint32_t _env;
};