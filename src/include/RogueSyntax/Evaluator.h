#pragma once

#include "StandardLib.h"
#include "IObject.h"
#include "Token.h"
#include "AstNode.h"

class Evaluator
{
public:
	std::shared_ptr<IObject> Eval(const std::shared_ptr<Program>& program, const std::shared_ptr<Environment>& env) const;
	virtual std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, const std::shared_ptr<Environment>& env) const = 0;

protected:
	std::shared_ptr<IObject> EvalPrefixExpression(const Token& op, const std::shared_ptr<IObject>& right) const;
	std::shared_ptr<IObject> EvalBangPrefixOperatorExpression(const Token& op, const std::shared_ptr<IObject>& right) const;
	std::shared_ptr<IObject> EvalMinusPrefixOperatorExpression(const Token& op, const std::shared_ptr<IObject>& right) const;

	std::shared_ptr<IObject> EvalInfixExpression(const Token& op, const std::shared_ptr<IObject>& left, const std::shared_ptr<IObject>& right) const;

	std::shared_ptr<IObject> EvalIntegerInfixExpression(const Token& op, const IntegerObj* const left, const IntegerObj* const right) const;
	std::shared_ptr<IObject> EvalBooleanInfixExpression(const Token& op, const BooleanObj* const left, const BooleanObj* const right) const;

	std::shared_ptr<IObject> EvalAsBoolean(const Token& context, const std::shared_ptr<IObject>& obj) const;

	std::shared_ptr<IObject> MakeError(const std::string& message, const Token& token) const;
	std::shared_ptr<IObject> UnwrapIfReturnObj(const std::shared_ptr<IObject>& input) const;

	std::shared_ptr<Environment> ExtendFunctionEnv(const std::shared_ptr<FunctionObj>& fn, const std::vector<std::shared_ptr<IObject>>& args) const;
};

class StackEvaluator : public Evaluator
{
public:
	std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, const std::shared_ptr<Environment>& env) const override;
};

class RecursiveEvaluator : public Evaluator
{
public:
	std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, const std::shared_ptr<Environment>& env) const override;

private:
	std::vector<std::shared_ptr<IObject>> EvalExpressions(const std::vector<std::shared_ptr<IExpression>>& expressions, const std::shared_ptr<Environment>& env) const;
	std::shared_ptr<IObject> ApplyFunction(const std::shared_ptr<FunctionObj>& fn, const std::vector<std::shared_ptr<IObject>>& args) const;
};