#pragma once

#include "StandardLib.h"
#include "IObject.h"
#include "Token.h"
#include "AstNode.h"

class Evaluator
{
public:
	std::shared_ptr<IObject> Eval(Program& program, std::shared_ptr<Environment>& env) const;
	virtual std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, std::shared_ptr<Environment>& env) const = 0;

protected:
	std::shared_ptr<IObject> EvalPrefixExpression(const Token& op, const std::shared_ptr<IObject>& right) const;
	std::shared_ptr<IObject> EvalBangPrefixOperatorExpression(const Token& op, const std::shared_ptr<IObject>& right) const;
	std::shared_ptr<IObject> EvalMinusPrefixOperatorExpression(const Token& op, const std::shared_ptr<IObject>& right) const;

	std::shared_ptr<IObject> EvalInfixExpression(const Token& op, const std::shared_ptr<IObject>& left, const std::shared_ptr<IObject>& right) const;
	std::shared_ptr<IObject> EvalIntegerInfixExpression(const Token& op, const std::shared_ptr<IntegerObj>& left, const std::shared_ptr<IntegerObj>& right) const;
	std::shared_ptr<IObject> EvalBooleanInfixExpression(const Token& op, const std::shared_ptr<BooleanObj>& left, const std::shared_ptr<BooleanObj>& right) const;

	std::shared_ptr<IObject> EvalAsBoolean(const Token& context, const std::shared_ptr<IObject>& obj) const;

	std::shared_ptr<IObject> MakeError(const std::string& message, const Token& token) const;
	std::shared_ptr<IObject> UnwrapIfReturnObj(const std::shared_ptr<IObject>& input) const;

	std::shared_ptr<Environment> ExtendFunctionEnv(std::shared_ptr<FunctionObj> fn, std::vector<std::shared_ptr<IObject>>& args) const;	
};

class StackEvaluator : public Evaluator
{
public:
	std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, std::shared_ptr<Environment>& env) const override;
};

class RecursiveEvaluator : public Evaluator
{
public:
	std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, std::shared_ptr<Environment>& env) const override;

private:
	std::vector<std::shared_ptr<IObject>> EvalExpressions(std::vector<std::shared_ptr<IExpression>>& expressions, std::shared_ptr<Environment>& env) const;
	std::shared_ptr<IObject> ApplyFunction(std::shared_ptr<FunctionObj> fn, std::vector<std::shared_ptr<IObject>>& args) const;
};