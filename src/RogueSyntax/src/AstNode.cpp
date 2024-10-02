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

std::shared_ptr<Program> Program::New(const std::shared_ptr<AstNodeStore>& store)
{
	return std::make_shared<Program>(store);
}

void Program::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void Program::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

LetStatement::LetStatement(const Token& token, const IExpression* name, const IExpression* value) : IStatement(token), Name(name), Value(value)
{
	SetUniqueId(this);
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

void LetStatement::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void LetStatement::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}


Identifier::Identifier(const ::Token& token, const std::string& value) : IExpression(token), Value(value)
{
	SetUniqueId(this);
}

std::string Identifier::ToString() const
{
	return Value;
}

void Identifier::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void Identifier::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}


ReturnStatement::ReturnStatement(const Token& token, const IExpression* returnValue) : IStatement(token), ReturnValue(returnValue)
{
	SetUniqueId(this);
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

void ReturnStatement::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}
void ReturnStatement::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

ExpressionStatement::ExpressionStatement(const Token& token, const IExpression* expression) : IStatement(token), Expression(expression)
{
	SetUniqueId(this);
}

std::string ExpressionStatement::ToString() const
{
	if (Expression != nullptr)
	{
		return Expression->ToString();
	}
	return "";
}

void ExpressionStatement::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}
void ExpressionStatement::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

NullLiteral::NullLiteral(const ::Token& token) : IExpression(token)
{
	SetUniqueId(this);
}


std::string NullLiteral::ToString() const
{
	return "null";
}

void NullLiteral::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void NullLiteral::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

IntegerLiteral::IntegerLiteral(const ::Token& token, int value) : IExpression(token), Value(value)
{
	SetUniqueId(this);
}

std::string IntegerLiteral::ToString() const
{
	return std::to_string(Value);
}

void IntegerLiteral::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}
void IntegerLiteral::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

BooleanLiteral::BooleanLiteral(const ::Token& token, bool value) : IExpression(token), Value(value)
{
	SetUniqueId(this);
}



std::string BooleanLiteral::ToString() const
{
	return Value ? "true" : "false";
}

void BooleanLiteral::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}
void BooleanLiteral::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

HashLiteral::HashLiteral(const Token& token, const std::map<IExpression*, IExpression*>& pairs) : IExpression(token), Elements(pairs)
{
	SetUniqueId(this);
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

void HashLiteral::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void HashLiteral::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

PrefixExpression::PrefixExpression(const Token& token, const std::string& op, const IExpression* right) : IExpression(token), Operator(op), Right(right)
{
	SetUniqueId(this);
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

void PrefixExpression::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}
void PrefixExpression::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

InfixExpression::InfixExpression(const Token& token, const IExpression* left, const std::string& op, const IExpression* right) : IExpression(token), Operator(op), Left(left), Right(right)
{
	SetUniqueId(this);
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

void InfixExpression::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void InfixExpression::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

BlockStatement::BlockStatement(const ::Token& token, const std::vector<IStatement*>& statements) : IStatement(token), Statements(statements)
{
	SetUniqueId(this);
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

void BlockStatement::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void BlockStatement::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

IfStatement::IfStatement(const Token& token, const IExpression* condition, const IStatement* consequence, const IStatement* alternative) : IStatement(token), Condition(condition), Consequence(consequence), Alternative(alternative)
{
	SetUniqueId(this);
}

std::string IfStatement::ToString() const
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

void IfStatement::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void IfStatement::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

FunctionLiteral::FunctionLiteral(const Token& token, const std::vector<IExpression*>& parameters, const IStatement* body) : IExpression(token), Parameters(parameters), Body(body), Name("")
{
	SetUniqueId(this);
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

void FunctionLiteral::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void FunctionLiteral::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

CallExpression::CallExpression(const Token& token, const IExpression* function, const std::vector<IExpression*>& arguments) : IExpression(token), Function(function), Arguments(arguments)
{
	SetUniqueId(this);
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

void CallExpression::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void CallExpression::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

WhileStatement::WhileStatement(const Token& token, const IExpression* condition, const IStatement* action) : IStatement(token), Condition(condition), Action(action)
{
	SetUniqueId(this);
}


std::string WhileStatement::ToString() const
{
	std::string result;
	result.append("while");
	result.append(Condition->ToString());
	result.append(" ");
	result.append(Action->ToString());
	return result;
}

void WhileStatement::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void WhileStatement::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

BreakStatement::BreakStatement(const ::Token& token) : IStatement(token)
{
	SetUniqueId(this);
}


std::string BreakStatement::ToString() const
{
	return "break";
}

void BreakStatement::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void BreakStatement::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

ContinueStatement::ContinueStatement(const ::Token& token) : IStatement(token)
{
	SetUniqueId(this);
}

std::string ContinueStatement::ToString() const
{
	return "continue";
}

void ContinueStatement::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}
void ContinueStatement::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

ForStatement::ForStatement(const Token& token, const IStatement* init, const IExpression* condition, const IStatement* post, const IStatement* action) : IStatement(token), Init(init), Condition(condition), Post(post), Action(action)
{
	SetUniqueId(this);
}

std::string ForStatement::ToString() const
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

void ForStatement::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}
void ForStatement::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}

StringLiteral::StringLiteral(const Token& token, const std::string& value) : IExpression(token), Value(value)
{
	SetUniqueId(this);
}


std::string StringLiteral::ToString() const
{
	return Value;
}

void StringLiteral::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void StringLiteral::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}


DecimalLiteral::DecimalLiteral(const Token& token, float value) : IExpression(token), Value(value)
{
	SetUniqueId(this);
}


std::string DecimalLiteral::ToString() const
{
	return std::to_string(Value);
}

void DecimalLiteral::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void DecimalLiteral::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}


ArrayLiteral::ArrayLiteral(const Token& token, const std::vector<IExpression*>& elements) : IExpression(token), Elements(elements)
{
	SetUniqueId(this);
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

void ArrayLiteral::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}
void ArrayLiteral::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}


IndexExpression::IndexExpression(const Token& token, const IExpression* left, const IExpression* index) : IExpression(token), Left(left), Index(index)
{
	SetUniqueId(this);
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

void IndexExpression::Eval(Evaluator* evaluator) const
{
	evaluator->NodeEval(this);
}

void IndexExpression::Compile(Compiler* compiler) const
{
	compiler->NodeCompile(this);
}
