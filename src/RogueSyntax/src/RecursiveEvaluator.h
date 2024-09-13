#pragma once
#include <RogueSyntaxCore.h>


class RecursiveEvaluator : public Evaluator
{
public:
	std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, const std::shared_ptr<Environment>& env, const std::shared_ptr<BuiltIn>& builtIn) override;

private:
	std::vector<std::shared_ptr<IObject>> EvalExpressions(const std::vector<std::shared_ptr<IExpression>>& expressions, const std::shared_ptr<Environment>& env, const std::shared_ptr<BuiltIn>& builtIn) const;
	std::shared_ptr<IObject> ApplyFunction(const std::shared_ptr<FunctionObj>& func, const std::vector<std::shared_ptr<IObject>>& args, const std::shared_ptr<BuiltIn>& builtIn) const;
};