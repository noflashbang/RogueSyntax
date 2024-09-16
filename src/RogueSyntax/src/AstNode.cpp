#include "pch.h"
#include "Evaluator.h"

std::string Program::ToString() const
{
	std::string result;
	std::for_each(Statements.begin(), Statements.end(), [&result](const auto& statement)
	{
		result.append(statement->ToString());
	});

	return result;
}

std::shared_ptr<Program>  Program::New()
{
	return std::make_shared<Program>();
}

void Program::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

LetStatement::LetStatement(const Token& token, const std::shared_ptr<IExpression>& name, const std::shared_ptr<IExpression>& value) : IStatement(token), Name(name), Value(value)
{
}

std::shared_ptr<LetStatement> LetStatement::New(const Token& token, const std::shared_ptr<IExpression>& name, const std::shared_ptr<IExpression>& value)
{
	return std::make_shared<LetStatement>(token, name, value);
}

std::string LetStatement::ToString() const
{
	std::string result;
	result.append(TokenLiteral() + " ");
	result.append(Name->ToString());
	result.append(" = ");
	if (Value != nullptr)
	{
		result.append(Value->ToString());
	}
	result.append(";");
	return result;
}

void LetStatement::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

Identifier::Identifier(const ::Token& token, const std::string& value) : IExpression(token), Value(value)
{
}

std::shared_ptr<Identifier> Identifier::New(const Token& token, const std::string& value)
{
	return std::make_shared<Identifier>(token, value);
}

std::string Identifier::ToString() const
{
	return Value;
}

void Identifier::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

ReturnStatement::ReturnStatement(const Token& token, const std::shared_ptr<IExpression>& returnValue) : IStatement(token), ReturnValue(returnValue)
{
}

std::shared_ptr<ReturnStatement> ReturnStatement::New(const Token& token, const std::shared_ptr<IExpression>& returnValue)
{
	return std::make_shared<ReturnStatement>(token, returnValue);
}

std::string ReturnStatement::ToString() const
{
	std::string result;
	result.append(TokenLiteral() + " ");
	if (ReturnValue != nullptr)
	{
		result.append(ReturnValue->ToString());
	}
	result.append(";");
	return result;
}

void ReturnStatement::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

ExpressionStatement::ExpressionStatement(const Token& token, const std::shared_ptr<IExpression>& expression) : IStatement(token), Expression(expression)
{
}

std::shared_ptr<ExpressionStatement> ExpressionStatement::New(const Token& token, const std::shared_ptr<IExpression>& expression)
{
	return std::make_shared<ExpressionStatement>(token,  expression);
}

std::string ExpressionStatement::ToString() const
{
	if (Expression != nullptr)
	{
		return Expression->ToString();
	}
	return "";
}

void ExpressionStatement::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

NullLiteral::NullLiteral(const ::Token& token) : IExpression(token)
{
}

std::shared_ptr<NullLiteral> NullLiteral::New(const Token& token)
{
	return std::make_shared<NullLiteral>(token);
}

std::string NullLiteral::ToString() const
{
	return "null";
}

void NullLiteral::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

IntegerLiteral::IntegerLiteral(const ::Token& token, int value) : IExpression(token), Value(value)
{
}

std::shared_ptr<IntegerLiteral> IntegerLiteral::New(const Token& token, const int value)
{
	return std::make_shared<IntegerLiteral>(token, value);
};

std::string IntegerLiteral::ToString() const
{
	return std::to_string(Value);
}

void IntegerLiteral::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

BooleanLiteral::BooleanLiteral(const ::Token& token, bool value) : IExpression(token), Value(value)
{
}

std::shared_ptr<BooleanLiteral> BooleanLiteral::New(const Token& token, const bool value)
{
	return std::make_shared<BooleanLiteral>(token, value);
};


std::string BooleanLiteral::ToString() const
{
	return Value ? "true" : "false";
}

void BooleanLiteral::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

HashLiteral::HashLiteral(const Token& token, const std::map<std::shared_ptr<IExpression>, std::shared_ptr<IExpression>>& pairs) : IExpression(token), Elements(pairs)
{
}

std::shared_ptr<HashLiteral> HashLiteral::New(const Token& token, const std::map<std::shared_ptr<IExpression>, std::shared_ptr<IExpression>>& pairs)
{
	return std::make_shared<HashLiteral>(token, pairs);
}

std::string HashLiteral::ToString() const
{
	std::string result;
	result.append("{");
	std::for_each(Elements.begin(), Elements.end(), [&result](const auto& pair)
		{
			result.append(pair.first->ToString());
			result.append(": ");
			result.append(pair.second->ToString());
			result.append(", ");
		});

	//remove the last comma
	if (result.size() > 1)
	{
		result.pop_back();
		result.pop_back();
	}
	result.append("}");
	return result;
}

void HashLiteral::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

PrefixExpression::PrefixExpression(const Token& token, const std::string& op, const std::shared_ptr<IExpression>& right) : IExpression(token), Operator(op), Right(right)
{
}

std::shared_ptr<PrefixExpression> PrefixExpression::New(const Token& token, const std::string& op, const std::shared_ptr<IExpression>& right)
{
	return std::make_shared<PrefixExpression>(token, op, right);
}

std::string PrefixExpression::ToString() const
{
	std::string result;
	result.append("(");
	result.append(Operator);
	result.append(Right->ToString());
	result.append(")");
	return result;
}

void PrefixExpression::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

InfixExpression::InfixExpression(const Token& token, const std::shared_ptr<IExpression>& left, const std::string& op, const std::shared_ptr<IExpression>& right) : IExpression(token), Operator(op), Left(left), Right(right)
{
}

std::shared_ptr<InfixExpression> InfixExpression::New(const Token& token, const std::shared_ptr<IExpression>& left, const std::string& op, const std::shared_ptr<IExpression>& right)
{
	return std::make_shared<InfixExpression>(token, left, op, right);
}

std::string InfixExpression::ToString() const
{
	std::string result;
	result.append("(");
	result.append(Left->ToString());
	result.append(" " + Operator + " ");
	result.append(Right->ToString());
	result.append(")");
	return result;
}

void InfixExpression::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}


BlockStatement::BlockStatement(const ::Token& token, const std::vector<std::shared_ptr<IStatement>>& statements) : IStatement(token), Statements(statements)
{
}

std::shared_ptr<BlockStatement> BlockStatement::New(const Token& token, const std::vector<std::shared_ptr<IStatement>>& statements)
{
	return std::make_shared<BlockStatement>(token, statements);
}

std::string BlockStatement::ToString() const
{
	std::string result;
	result.append("{");
	std::for_each(Statements.begin(), Statements.end(), [&result](const auto& statement)
	{
		result.append(statement->ToString());
	});
	result.append("}");

	return result;
}

void BlockStatement::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

IfExpression::IfExpression(const Token& token, const std::shared_ptr<IExpression>& condition, const std::shared_ptr<IStatement>& consequence, const std::shared_ptr<IStatement>& alternative) : IExpression(token), Condition(condition), Consequence(consequence), Alternative(alternative)
{
}

std::shared_ptr<IfExpression> IfExpression::New(const Token& token, const std::shared_ptr<IExpression>& condition, const std::shared_ptr<IStatement>& consequence, const std::shared_ptr<IStatement>& alternative)
{
	return std::make_shared<IfExpression>(token, condition, consequence, alternative);
}

std::string IfExpression::ToString() const
{
	std::string result;
	result.append("if");
	result.append(Condition->ToString());
	result.append(" ");
	result.append(Consequence->ToString());
	if (Alternative != nullptr)
	{
		result.append("else ");
		result.append(Alternative->ToString());
	}
	return result;
}

void IfExpression::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

FunctionLiteral::FunctionLiteral(const Token& token, const std::vector<std::shared_ptr<IExpression>>& parameters, const std::shared_ptr<IStatement>& body) : IExpression(token), Parameters(parameters), Body(body)
{
}


std::shared_ptr<FunctionLiteral> FunctionLiteral::New(const Token& token, const std::vector<std::shared_ptr<IExpression>>& parameters, const std::shared_ptr<IStatement>& body)
{
	return std::make_shared<FunctionLiteral>(token, parameters, body);
}

std::string FunctionLiteral::ToString() const
{
	std::string result;
	result.append(TokenLiteral());
	result.append("(");
	std::for_each(Parameters.begin(), Parameters.end(), [&result](const auto& param)
	{
		result.append(param->ToString());
		result.append(", ");
	});

	//remove the last comma
	result.pop_back();
	result.pop_back();

	result.append(")");
	result.append(Body->ToString());
	return result;
}

void FunctionLiteral::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

CallExpression::CallExpression(const Token& token, const std::shared_ptr<IExpression>& function, const std::vector<std::shared_ptr<IExpression>>& arguments) : IExpression(token), Function(function), Arguments(arguments)
{
}

std::shared_ptr<CallExpression> CallExpression::New(const Token& token, const std::shared_ptr<IExpression>& function, const std::vector<std::shared_ptr<IExpression>>& arguments)
{
	return std::make_shared<CallExpression>(token, function, arguments);
}

std::string CallExpression::ToString() const
{
	std::string result;
	result.append(Function->ToString());
	result.append("(");
	std::for_each(Arguments.begin(), Arguments.end(), [&result](const auto& arg)
	{
		result.append(arg->ToString());
		result.append(", ");
	});

	//remove the last comma
	result.pop_back();
	result.pop_back();

	result.append(")");
	return result;
}

void CallExpression::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

WhileExpression::WhileExpression(const Token& token, const std::shared_ptr<IExpression>& condition, const std::shared_ptr<IStatement>& action) : IExpression(token), Condition(condition), Action(action)
{
}

std::shared_ptr<WhileExpression> WhileExpression::New(const Token& token, const std::shared_ptr<IExpression>& condition, const std::shared_ptr<IStatement>& action)
{
	return std::make_shared<WhileExpression>(token, condition, action);
}

std::string WhileExpression::ToString() const
{
	std::string result;
	result.append("while");
	result.append(Condition->ToString());
	result.append(" ");
	result.append(Action->ToString());
	return result;
}

void WhileExpression::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}


BreakStatement::BreakStatement(const ::Token& token) : IStatement(token)
{
}

std::shared_ptr<BreakStatement> BreakStatement::New(const ::Token& token)
{
	return std::make_shared<BreakStatement>(token);
}

std::string BreakStatement::ToString() const
{
	return "break";
}

void BreakStatement::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

ContinueStatement::ContinueStatement(const ::Token& token) : IStatement(token)
{
}

std::shared_ptr<ContinueStatement> ContinueStatement::New(const ::Token& token)
{
	return std::make_shared<ContinueStatement>(token);
}

std::string ContinueStatement::ToString() const
{
	return "continue";
}

void ContinueStatement::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

ForExpression::ForExpression(const Token& token, const std::shared_ptr<IStatement>& init, const std::shared_ptr<IExpression>& condition, const std::shared_ptr<IStatement>& post, const std::shared_ptr<IStatement>& action) : IExpression(token), Init(init), Condition(condition), Post(post), Action(action)
{
}

std::shared_ptr<ForExpression> ForExpression::New(const Token& token, const std::shared_ptr<IStatement>& init, const std::shared_ptr<IExpression>& condition, const std::shared_ptr<IStatement>& post, const std::shared_ptr<IStatement>& action)
{
	return std::make_shared<ForExpression>(token, init, condition, post, action);
}

std::string ForExpression::ToString() const
{
	std::string result;
	result.append("for (");
	result.append(Init->ToString());
	result.append(" ");
	result.append(Condition->ToString());
	result.append("; ");
	result.append(Post->ToString());
	if (result.ends_with(";"))
	{
		result.pop_back();
	}
	result.append(") ");
	result.append(Action->ToString());
	return result;
}

void ForExpression::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

StringLiteral::StringLiteral(const Token& token, const std::string& value) : IExpression(token), Value(value)
{
}

std::shared_ptr<StringLiteral> StringLiteral::New(const Token& token, const std::string& value)
{
	return std::make_shared<StringLiteral>(token, value);
}

std::string StringLiteral::ToString() const
{
	return Value;
}

void StringLiteral::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

DecimalLiteral::DecimalLiteral(const Token& token, float value) : IExpression(token), Value(value)
{
}

std::shared_ptr<DecimalLiteral> DecimalLiteral::New(const Token& token, float value)
{
	return std::make_shared<DecimalLiteral>(token, value);
}

std::string DecimalLiteral::ToString() const
{
	return std::to_string(Value);
}

void DecimalLiteral::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

ArrayLiteral::ArrayLiteral(const Token& token, const std::vector<std::shared_ptr<IExpression>>& elements) : IExpression(token), Elements(elements)
{
}

std::shared_ptr<ArrayLiteral> ArrayLiteral::New(const Token& token, const std::vector<std::shared_ptr<IExpression>>& elements)
{
	return std::make_shared<ArrayLiteral>(token, elements);
}

std::string ArrayLiteral::ToString() const
{
	std::string result;
	result.append("[");
	std::for_each(Elements.begin(), Elements.end(), [&result](const auto& element)
		{
			result.append(element->ToString());
			result.append(", ");
		});

	//remove the last comma
	result.pop_back();
	result.pop_back();

	result.append("]");
	return result;
}

void ArrayLiteral::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

IndexExpression::IndexExpression(const Token& token, const std::shared_ptr<IExpression>& left, const std::shared_ptr<IExpression>& index) : IExpression(token), Left(left), Index(index)
{
}

std::shared_ptr<IndexExpression> IndexExpression::New(const Token& token, const std::shared_ptr<IExpression>& left, const std::shared_ptr<IExpression>& index)
{
	return std::make_shared<IndexExpression>(token, left, index);
}

std::string IndexExpression::ToString() const
{
	std::string result;
	result.append("(");
	result.append(Left->ToString());
	result.append("[");
	result.append(Index->ToString());
	result.append("])");
	return result;
}

void IndexExpression::Eval(Evaluator* evaluator)
{
	evaluator->NodeEval(this);
}

