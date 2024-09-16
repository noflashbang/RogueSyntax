#pragma once

#include "StandardLib.h"
#include <RogueSyntaxCore.h>

enum class EvaluatorType
{
	Stack,
	Recursive
};

class Evaluator
{
public:
	Evaluator();
	std::shared_ptr<IObject> Eval(const std::shared_ptr<Program>& program);
	virtual std::shared_ptr<IObject> Eval(const std::shared_ptr<INode>& node, const uint32_t env) = 0;

	virtual void NodeEval(Program* program) = 0;
	virtual void NodeEval(BlockStatement* block) = 0;
	virtual void NodeEval(ExpressionStatement* expression) = 0;
	virtual void NodeEval(ReturnStatement* ret) = 0;
	virtual void NodeEval(LetStatement* let) = 0;
	virtual void NodeEval(Identifier* ident) = 0;
	virtual void NodeEval(IntegerLiteral* integer) = 0;
	virtual void NodeEval(BooleanLiteral* boolean) = 0;
	virtual void NodeEval(StringLiteral* string) = 0;
	virtual void NodeEval(DecimalLiteral* decimal) = 0;
	virtual void NodeEval(PrefixExpression* prefix) = 0;
	virtual void NodeEval(InfixExpression* infix) = 0;
	virtual void NodeEval(IfExpression* ifExpr) = 0;
	virtual void NodeEval(FunctionLiteral* function) = 0;
	virtual void NodeEval(CallExpression* call) = 0;
	virtual void NodeEval(ArrayLiteral* array) = 0;
	virtual void NodeEval(IndexExpression* index) = 0;
	virtual void NodeEval(HashLiteral* hash) = 0;
	virtual void NodeEval(NullLiteral* null) = 0;
	virtual void NodeEval(WhileExpression* whileExp) = 0;
	virtual void NodeEval(ForExpression* forExp) = 0;
	virtual void NodeEval(ContinueStatement* cont) = 0;
	virtual void NodeEval(BreakStatement* brk)=0;

	static std::shared_ptr<Evaluator> New(EvaluatorType type);

	virtual std::string Type() = 0;

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

	std::shared_ptr<Environment> EvalEnvironment;
	std::shared_ptr<BuiltIn> EvalBuiltIn;

	static std::shared_ptr<IObject> MakeError(const std::string& message, const Token& token);
	static std::shared_ptr<IObject> UnwrapIfReturnObj(const std::shared_ptr<IObject>& input);
	static std::shared_ptr<IObject> UnwrapIfIdentObj(const std::shared_ptr<IObject>& input);

	uint32_t ExtendFunctionEnv(const uint32_t rootEnv, const std::shared_ptr<FunctionObj>& func, const std::vector<std::shared_ptr<IObject>>& args);


private:
	std::map<ObjectType, std::function<std::shared_ptr<IObject>(const Token& context, const IObject* const right)>> _coercionMap;
	std::map<ObjectType, std::map<ObjectType, ObjectType>> _coercionTable;
};

