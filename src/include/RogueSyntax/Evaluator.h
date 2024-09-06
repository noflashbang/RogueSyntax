#pragma once

#include "StandardLib.h"
#include "IObject.h"
#include "Token.h"
#include "AstNode.h"

class Evaluator
{
public:
	IObject* Eval(Program& program, Environment* env);
	IObject* Eval(INode* node, Environment* env);

private:

	IObject* EvalPrefixExpression(const Token& op, IObject* right);
	IObject* EvalInfixExpression(const Token& op, IObject* left, IObject* right);

	IObject* EvalIntegerInfixExpression(const Token& op, IntegerObj* left, IntegerObj* right);
	IObject* EvalBooleanInfixExpression(const Token& op, BooleanObj* left, BooleanObj* right);

	bool IsTruthy(IObject* obj);
	BooleanObj* EvalAsBoolean(IObject* obj);

	IObject* EvalBangOperatorExpression(const Token& op, IObject* right);
	IObject* EvalMinusPrefixOperatorExpression(const Token& op, IObject* right);

	IObject* MakeError(const std::string& message, const Token& token);

	std::vector<IObject*> EvalExpressions(std::vector<std::shared_ptr<IExpression>>& expressions, Environment* env);
	IObject* ApplyFunction(FunctionObj* fn, std::vector<IObject*>& args);
	Environment* ExtendFunctionEnv(FunctionObj* fn, std::vector<IObject*>& args);
};

