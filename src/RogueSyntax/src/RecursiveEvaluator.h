#pragma once
#include <RogueSyntaxCore.h>


class RecursiveEvaluator : public Evaluator
{
public:
	std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, const uint32_t env) override;

private:
	std::vector<std::shared_ptr<IObject>> EvalExpressions(const std::vector<std::shared_ptr<IExpression>>& expressions, const uint32_t env);
	std::shared_ptr<IObject> ApplyFunction(const uint32_t env, const std::shared_ptr<FunctionObj>& func, const std::vector<std::shared_ptr<IObject>>& args);
};