#pragma once

#include "StandardLib.h"
#include "IObject.h"
#include "Token.h"
#include "AstNode.h"
#include "Environment.h"
#include "BuiltIn.h"

class Evaluator
{
public:
	Evaluator();
	std::shared_ptr<IObject> Eval(const std::shared_ptr<Program>& program, const std::shared_ptr<Environment>& env, const std::shared_ptr<BuiltIn>& builtIn) const;
	virtual std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, const std::shared_ptr<Environment>& env, const std::shared_ptr<BuiltIn>& builtIn) const = 0;

protected:
	std::shared_ptr<IObject> EvalPrefixExpression(const Token& op, const std::shared_ptr<IObject>& right) const;
	std::shared_ptr<IObject> EvalBangPrefixOperatorExpression(const Token& op, const std::shared_ptr<IObject>& right) const;
	std::shared_ptr<IObject> EvalMinusPrefixOperatorExpression(const Token& op, const std::shared_ptr<IObject>& right) const;
	std::shared_ptr<IObject> EvalBitwiseNotPrefixOperatorExpression(const Token& optor, const std::shared_ptr<IObject>& right) const;

	std::shared_ptr<IObject> EvalInfixExpression(const Token& op, const std::shared_ptr<IObject>& left, const std::shared_ptr<IObject>& right) const;
	std::shared_ptr<IObject> EvalIndexExpression(const Token& op, const std::shared_ptr<IObject>& operand, const std::shared_ptr<IObject>& index) const;

	std::shared_ptr<IObject> EvalNullInfixExpression(const Token& op, const IObject* const left, const IObject* const right) const;
	std::shared_ptr<IObject> EvalIntegerInfixExpression(const Token& op, const IntegerObj* const left, const IntegerObj* const right) const;
	std::shared_ptr<IObject> EvalBooleanInfixExpression(const Token& op, const BooleanObj* const left, const BooleanObj* const right) const;
	std::shared_ptr<IObject> EvalDecimalInfixExpression(const Token& optor, const DecimalObj* const left, const DecimalObj* const right) const;
	std::shared_ptr<IObject> EvalStringInfixExpression(const Token& optor, const StringObj* const left, const StringObj* const right) const;

	//convertion functions - the left hand type is the type of the object that the coercion is being applied to
	bool CanCoerceTypes(const IObject* const left, const IObject* const right) const;
	std::tuple<std::shared_ptr<IObject>, std::shared_ptr<IObject>> CoerceTypes(const Token& context, const IObject* const left, const IObject* const right) const;
	std::shared_ptr<IObject>  CoerceThis(const Token& context, const IObject* const source, const IObject* const target) const;

	std::shared_ptr<IObject> EvalAsBoolean(const Token& context, const IObject* const obj) const;
	std::shared_ptr<IObject> EvalAsDecimal(const Token& context, const IObject* const obj) const;
	std::shared_ptr<IObject> EvalAsInteger(const Token& context, const IObject* const obj) const;

	static std::shared_ptr<IObject> MakeError(const std::string& message, const Token& token);
	static std::shared_ptr<IObject> UnwrapIfReturnObj(const std::shared_ptr<IObject>& input);
	static std::shared_ptr<IObject> UnwrapIfIdentObj(const std::shared_ptr<IObject>& input);

	static std::shared_ptr<Environment> ExtendFunctionEnv(const std::shared_ptr<FunctionObj>& func, const std::vector<std::shared_ptr<IObject>>& args);

private:
	std::map<ObjectType, std::function<std::shared_ptr<IObject>(const Token& context, const IObject* const right)>> _coercionMap;
	std::map<ObjectType, std::map<ObjectType, ObjectType>> _coercionTable;
};

class StackEvaluator : public Evaluator
{
public:
	std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, const std::shared_ptr<Environment>& env, const std::shared_ptr<BuiltIn>& builtIn) const override;
};

class RecursiveEvaluator : public Evaluator
{
public:
	std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, const std::shared_ptr<Environment>& env, const std::shared_ptr<BuiltIn>& builtIn) const override;

private:
	std::vector<std::shared_ptr<IObject>> EvalExpressions(const std::vector<std::shared_ptr<IExpression>>& expressions, const std::shared_ptr<Environment>& env, const std::shared_ptr<BuiltIn>& builtIn) const;
	std::shared_ptr<IObject> ApplyFunction(const std::shared_ptr<FunctionObj>& func, const std::vector<std::shared_ptr<IObject>>& args, const std::shared_ptr<BuiltIn>& builtIn) const;
};