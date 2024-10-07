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
	Evaluator(const std::shared_ptr<ObjectFactory> factory);
	const IObject* Eval(const std::shared_ptr<Program>& program, const std::shared_ptr<BuiltIn>& externs);
	static std::shared_ptr<Evaluator> New(EvaluatorType type, const std::shared_ptr<ObjectFactory>& factory);
	virtual std::string Type() = 0;
	
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
	virtual void NodeEval(const BreakStatement* brk) = 0;

protected:

	const IObject* Eval(const std::shared_ptr<Program>& program, const uint32_t env);
	virtual const IObject* Eval(const INode* node, const uint32_t env) = 0;

	uint32_t MakeEnv();
	void FreeEnv(const uint32_t env);




protected:
	const IObject* EvalPrefixExpression(const uint32_t env, const Token& op, const IObject* right) const;
	const IObject* EvalBangPrefixOperatorExpression(const uint32_t env, const Token& op, const IObject* right) const;
	const IObject* EvalMinusPrefixOperatorExpression(const uint32_t env, const Token& op, const IObject* right) const;
	const IObject* EvalBitwiseNotPrefixOperatorExpression(const uint32_t env, const Token& optor, const IObject* right) const;

	const IObject* EvalInfixExpression(const uint32_t env, const Token& op, const IObject* left, const IObject* right) const;
	const IObject* EvalIndexExpression(const uint32_t env, const Token& op, const IObject* operand, const IObject* index) const;

	const IObject* EvalNullInfixExpression(const uint32_t env, const Token& op, const IObject* const left, const IObject* const right) const;
	const IObject* EvalIntegerInfixExpression(const uint32_t env, const Token& op, const IntegerObj* const left, const IntegerObj* const right) const;
	const IObject* EvalBooleanInfixExpression(const uint32_t env, const Token& op, const BooleanObj* const left, const BooleanObj* const right) const;
	const IObject* EvalDecimalInfixExpression(const uint32_t env, const Token& optor, const DecimalObj* const left, const DecimalObj* const right) const;
	const IObject* EvalStringInfixExpression(const uint32_t env, const Token& optor, const StringObj* const left, const StringObj* const right) const;

	const IObject* EvalAsBoolean(const uint32_t env, const Token& context, const IObject* const obj) const;
	const IObject* EvalAsDecimal(const uint32_t env, const Token& context, const IObject* const obj) const;
	const IObject* EvalAsInteger(const uint32_t env, const Token& context, const IObject* const obj) const;

	std::shared_ptr<Environment> EvalEnvironment;
	std::shared_ptr<BuiltIn> EvalBuiltIn;
	std::shared_ptr<ObjectFactory> EvalFactory;

	static const IObject* UnwrapIfReturnObj(const IObject* input);
	static const IObject* UnwrapIfIdentObj(const IObject* input);

	uint32_t ExtendFunctionEnv(const uint32_t rootEnv, const FunctionObj* func, const std::vector<const IObject*>& args);

	inline IObject* MakeError(const uint32_t env, const std::string& message, const Token& token) const
	{
		return EvalFactory->New<ErrorObj>(message, token);
	};

private:
	TypeCoercer _coercer;
};

