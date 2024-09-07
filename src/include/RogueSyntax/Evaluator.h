#pragma once

#include "StandardLib.h"
#include "IObject.h"
#include "Token.h"
#include "AstNode.h"

class Evaluator
{
public:
	std::shared_ptr<IObject> Eval(Program& program, std::shared_ptr<Environment>& env);
	std::shared_ptr<IObject> StackEval(const std::shared_ptr<INode>& node, std::shared_ptr<Environment>& env);
	std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, std::shared_ptr<Environment>& env);

private:

	std::shared_ptr<IObject> EvalPrefixExpression(const Token& op, const std::shared_ptr<IObject>& right);
	std::shared_ptr<IObject> EvalInfixExpression(const Token& op, const std::shared_ptr<IObject>& left, const std::shared_ptr<IObject>& right);

	std::shared_ptr<IObject> EvalIntegerInfixExpression(const Token& op, const std::shared_ptr<IntegerObj>& left, const std::shared_ptr<IntegerObj>& right);
	std::shared_ptr<IObject> EvalBooleanInfixExpression(const Token& op, const std::shared_ptr<BooleanObj>& left, const std::shared_ptr<BooleanObj>& right);

	bool IsTruthy(const std::shared_ptr<IObject>& obj) const;
	std::shared_ptr<BooleanObj> EvalAsBoolean(const std::shared_ptr<IObject>& obj) const;

	std::shared_ptr<IObject> EvalBangOperatorExpression(const Token& op, const std::shared_ptr<IObject>& right);
	std::shared_ptr<IObject> EvalMinusPrefixOperatorExpression(const Token& op, const std::shared_ptr<IObject>& right);

	std::shared_ptr<IObject> MakeError(const std::string& message, const Token& token);

	std::vector<std::shared_ptr<IObject>> EvalExpressions(std::vector<std::shared_ptr<IExpression>>& expressions, std::shared_ptr<Environment>& env);
	std::shared_ptr<IObject> ApplyFunction(std::shared_ptr<FunctionObj> fn, std::vector<std::shared_ptr<IObject>>& args);
	std::shared_ptr<Environment> ExtendFunctionEnv(std::shared_ptr<FunctionObj> fn, std::vector<std::shared_ptr<IObject>>& args);
};

