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
	std::shared_ptr<IObject> Eval(const std::shared_ptr<Program>& program, const uint32_t env);
	virtual std::shared_ptr<IObject> Eval(const INode* node, const uint32_t env) = 0;

	uint32_t MakeEnv();
	void FreeEnv(const uint32_t env);

	virtual void NodeEval(const Program* program) = 0;
	virtual void NodeEval(const BlockStatement* block) = 0;
	virtual void NodeEval(const ExpressionStatement* expression) = 0;
	virtual void NodeEval(const ReturnStatement* ret) = 0;
	virtual void NodeEval(const LetStatement* let) = 0;
	virtual void NodeEval(const Identifier* ident) = 0;
	virtual void NodeEval(const IntegerLiteral* integer) = 0;
	virtual void NodeEval(const BooleanLiteral* boolean) = 0;
	virtual void NodeEval(const StringLiteral* string) = 0;
	virtual void NodeEval(const DecimalLiteral* decimal) = 0;
	virtual void NodeEval(const PrefixExpression* prefix) = 0;
	virtual void NodeEval(const InfixExpression* infix) = 0;
	virtual void NodeEval(const IfStatement* ifExpr) = 0;
	virtual void NodeEval(const FunctionLiteral* function) = 0;
	virtual void NodeEval(const CallExpression* call) = 0;
	virtual void NodeEval(const ArrayLiteral* array) = 0;
	virtual void NodeEval(const IndexExpression* index) = 0;
	virtual void NodeEval(const HashLiteral* hash) = 0;
	virtual void NodeEval(const NullLiteral* null) = 0;
	virtual void NodeEval(const WhileStatement* whileExp) = 0;
	virtual void NodeEval(const ForStatement* forExp) = 0;
	virtual void NodeEval(const ContinueStatement* cont) = 0;
	virtual void NodeEval(const BreakStatement* brk)=0;

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

	std::shared_ptr<IObject> EvalAsBoolean(const Token& context, const IObject* const obj) const;
	std::shared_ptr<IObject> EvalAsDecimal(const Token& context, const IObject* const obj) const;
	std::shared_ptr<IObject> EvalAsInteger(const Token& context, const IObject* const obj) const;

	std::shared_ptr<Environment> EvalEnvironment;
	std::shared_ptr<BuiltIn> EvalBuiltIn;


	static std::shared_ptr<IObject> UnwrapIfReturnObj(const std::shared_ptr<IObject>& input);
	static std::shared_ptr<IObject> UnwrapIfIdentObj(const std::shared_ptr<IObject>& input);

	uint32_t ExtendFunctionEnv(const uint32_t rootEnv, const std::shared_ptr<FunctionObj>& func, const std::vector<std::shared_ptr<IObject>>& args);


private:
	TypeCoercer _coercer;
};

