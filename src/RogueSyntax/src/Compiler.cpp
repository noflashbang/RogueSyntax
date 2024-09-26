#include <pch.h>

Compiler::Compiler()
{
}

Compiler::~Compiler()
{
}

CompilerError Compiler::Compile(std::shared_ptr<INode> node)
{
	return Compile(node.get());
}

CompilerError Compiler::Compile(INode* node)
{
	node->Compile(this);
	if (!_errorStack.empty())
	{
		return _errorStack.top().Error;
	}
}

ByteCode Compiler::GetByteCode() const
{
	return ByteCode {_instructions, _constants };
}

int Compiler::AddConstant(std::shared_ptr<IObject> obj)
{
	_constants.push_back(obj);
	return _constants.size() - 1;
}

int Compiler::Emit(OpCode::Constants opcode, std::vector<int> operands)
{
	auto instructions = OpCode::Make(opcode, operands);
	return AddInstruction(instructions);
}

int Compiler::AddInstruction(Instructions instructions)
{
	auto position = _instructions.size();
	_instructions.insert(_instructions.end(), instructions.begin(), instructions.end());
	return position;
}

void Compiler::NodeCompile(Program* program)
{
	for (auto stmt : program->Statements)
	{
		stmt->Compile(this);
		if (HasErrors())
		{
			return;
		}
	}
}

void Compiler::NodeCompile(BlockStatement* block)
{
	for (auto stmt : block->Statements)
	{
		stmt->Compile(this);
		if (HasErrors())
		{
			return;
		}
	}
}

void Compiler::NodeCompile(ExpressionStatement* expression)
{
	expression->Expression->Compile(this);
	if (HasErrors())
	{
		return;
	}
	Emit(OpCode::Constants::OP_POP, {});
}

void Compiler::NodeCompile(ReturnStatement* ret)
{
}

void Compiler::NodeCompile(LetStatement* let)
{
}

void Compiler::NodeCompile(Identifier* ident)
{
}

void Compiler::NodeCompile(IntegerLiteral* integer)
{
	auto obj = IntegerObj::New(integer->Value);
	auto index = AddConstant(obj);
	Emit(OpCode::Constants::OP_CONSTANT, { index });
}

void Compiler::NodeCompile(BooleanLiteral* boolean)
{
	boolean->Value ? Emit(OpCode::Constants::OP_TRUE, {}) : Emit(OpCode::Constants::OP_FALSE, {});
}

void Compiler::NodeCompile(StringLiteral* string)
{
	auto obj = StringObj::New(string->Value);
	auto index = AddConstant(obj);
	Emit(OpCode::Constants::OP_CONSTANT, { index });
}

void Compiler::NodeCompile(DecimalLiteral* decimal)
{
	auto obj = DecimalObj::New(decimal->Value);
	auto index = AddConstant(obj);
	Emit(OpCode::Constants::OP_CONSTANT, { index });
}

void Compiler::NodeCompile(PrefixExpression* prefix)
{
	prefix->Right->Compile(this);
	if (HasErrors())
	{
		return;
	}

	if (prefix->Operator == "-")
	{
		Emit(OpCode::Constants::OP_NEGATE, {});
	}
	else if (prefix->Operator == "!")
	{
		Emit(OpCode::Constants::OP_NOT, {});
	}
	else if (prefix->Operator == "~")
	{
		Emit(OpCode::Constants::OP_BNOT, {});
	}
	else
	{
		_errorStack.push(CompilerErrorInfo::New(CompilerError::UnknownOperator, std::format("Unknown operator {}", prefix->Operator)));
	}
}

void Compiler::NodeCompile(InfixExpression* infix)
{
	infix->Left->Compile(this);
	if (HasErrors())
	{
		return;
	}

	infix->Right->Compile(this);
	if (HasErrors())
	{
		return;
	}

	if (infix->Operator == "+")
	{
		Emit(OpCode::Constants::OP_ADD, {});
	}
	else if (infix->Operator == "-")
	{
		Emit(OpCode::Constants::OP_SUB, {});
	}
	else if (infix->Operator == "*")
	{
		Emit(OpCode::Constants::OP_MUL, {});
	}
	else if (infix->Operator == "/")
	{
		Emit(OpCode::Constants::OP_DIV, {});
	}
	else if (infix->Operator == "%")
	{
		Emit(OpCode::Constants::OP_MOD, {});
	}
	else if (infix->Operator == "|")
	{
		Emit(OpCode::Constants::OP_BOR, {});
	}
	else if (infix->Operator == "&")
	{
		Emit(OpCode::Constants::OP_BAND, {});
	}
	else if (infix->Operator == "^")
	{
		Emit(OpCode::Constants::OP_BXOR, {});
	}
	else if (infix->Operator == "<<")
	{
		Emit(OpCode::Constants::OP_BLSHIFT, {});
	}
	else if (infix->Operator == ">>")
	{
		Emit(OpCode::Constants::OP_BRSHIFT, {});
	}
	else if (infix->Operator == "==")
	{
		Emit(OpCode::Constants::OP_EQUAL, {});
	}
	else if (infix->Operator == "!=")
	{
		Emit(OpCode::Constants::OP_NOT_EQUAL, {});
	}
	else if (infix->Operator == ">")
	{
		Emit(OpCode::Constants::OP_GREATER_THAN, {});
	}
	else if (infix->Operator == ">=")
	{
		Emit(OpCode::Constants::OP_GREATER_THAN_EQUAL, {});
	}
	else if (infix->Operator == "<")
	{
		Emit(OpCode::Constants::OP_LESS_THAN, {});
	}
	else if (infix->Operator == "<=")
	{
		Emit(OpCode::Constants::OP_LESS_THAN_EQUAL, {});
	}
	else if (infix->Operator == "&&")
	{
		Emit(OpCode::Constants::OP_BOOL_AND, {});
	}
	else if (infix->Operator == "||")
	{
		Emit(OpCode::Constants::OP_BOOL_OR, {});
	}
	else
	{
		_errorStack.push(CompilerErrorInfo::New(CompilerError::UnknownOperator, std::format("Unknown operator {}", infix->Operator)));
	}
}

void Compiler::NodeCompile(IfExpression* ifExpr)
{
}

void Compiler::NodeCompile(FunctionLiteral* function)
{
}

void Compiler::NodeCompile(CallExpression* call)
{
}

void Compiler::NodeCompile(ArrayLiteral* array)
{
}

void Compiler::NodeCompile(IndexExpression* index)
{
}

void Compiler::NodeCompile(HashLiteral* hash)
{
}

void Compiler::NodeCompile(NullLiteral* null)
{
	Emit(OpCode::Constants::OP_NULL, {});
}

void Compiler::NodeCompile(WhileExpression* whileExp)
{
}

void Compiler::NodeCompile(ForExpression* forExp)
{
}

void Compiler::NodeCompile(ContinueStatement* cont)
{
}

void Compiler::NodeCompile(BreakStatement* brk)
{
}

std::shared_ptr<Compiler> Compiler::New()
{
	return std::make_shared<Compiler>();
}

