#include <pch.h>


SymbolTable::SymbolTable(std::string scope)
	: _scope(scope)
{
}

SymbolTable::SymbolTable()
	: _scope(SCOPE_GLOBAL)
{
}

Symbol SymbolTable::Define(const std::string& name)
{
	auto current = Resolve(name);
	if (current.Index != -1)
	{
		return current;
	}

	auto symbol = Symbol{ name, _scope, static_cast<int>(_store.size()) };
	_store.push_back(symbol);
	return symbol;
}

Symbol SymbolTable::Resolve(const std::string& name)
{
	auto symbol = Symbol{ name, _scope, -1 };
	for (int i = _store.size() - 1; i >= 0; i--)
	{
		if (_store[i].Name == name)
		{
			symbol = _store[i];
		}
	}
	return symbol;
}

Compiler::Compiler()
{
	_globalSymbolTable = SymbolTable::New(SCOPE_GLOBAL);
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

	SetLastInstruction(instructions);
	return position;
}

void Compiler::RemoveLastPop()
{
	if (LastInstructionIs(OpCode::Constants::OP_POP))
	{
		RemoveLastInstruction();
	}
}

bool Compiler::LastInstructionIs(OpCode::Constants opcode)
{
	return _lastInstruction[0] == static_cast<uint8_t>(opcode);
}

void Compiler::RemoveLastInstruction()
{
	_instructions.resize(_instructions.size() - _lastInstruction.size());
	SetLastInstruction(_previousLastInstruction);
}

void Compiler::ChangeOperand(int position, int operand)
{
	auto instruction = OpCode::Make(static_cast<OpCode::Constants>(_instructions[position]), { operand });
	ReplaceInstruction(position, instruction);
}

void Compiler::ReplaceInstruction(int position, Instructions instructions)
{
	for (const auto& instr : instructions)
	{
		_instructions[position++] = instr;
	}
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
	let->Value->Compile(this);
	if (HasErrors())
	{
		return;
	}
	if (typeid(*(let->Name.get())) == typeid(Identifier))
	{
		auto ident = dynamic_cast<Identifier*>(let->Name.get());
		auto symbol = _globalSymbolTable->Define(ident->Value);
		Emit(OpCode::Constants::OP_SET_GLOBAL, { symbol.Index });
	}
}

void Compiler::NodeCompile(Identifier* ident)
{
	auto symbol = _globalSymbolTable->Resolve(ident->Value);
	if (symbol.Scope == SCOPE_GLOBAL)
	{
		Emit(OpCode::Constants::OP_GET_GLOBAL, { symbol.Index });
	}
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
	ifExpr->Condition->Compile(this);
	if (HasErrors())
	{
		return;
	}

	auto jumpNotTruthyPos = Emit(OpCode::Constants::OP_JUMP_IF_FALSE, { 9999 });
	//
	ifExpr->Consequence->Compile(this);
	if (HasErrors())
	{
		return;
	}

	RemoveLastPop();
	auto jumpPos = Emit(OpCode::Constants::OP_JUMP, { 9999 });

	auto afterConsequencePos = _instructions.size();
	ChangeOperand(jumpNotTruthyPos, afterConsequencePos);
	
	if (ifExpr->Alternative != nullptr)
	{
		ifExpr->Alternative->Compile(this);
		if (HasErrors())
		{
			return;
		}
		RemoveLastPop();
	}
	else
	{
		Emit(OpCode::Constants::OP_NULL, {});
	}

	auto afterAlternativePos = _instructions.size();
	ChangeOperand(jumpPos, afterAlternativePos);
}

void Compiler::NodeCompile(FunctionLiteral* function)
{
}

void Compiler::NodeCompile(CallExpression* call)
{
}

void Compiler::NodeCompile(ArrayLiteral* array)
{
	for(auto elem : array->Elements)
	{
		elem->Compile(this);
		if (HasErrors())
		{
			return;
		}
	}
	Emit(OpCode::Constants::OP_ARRAY, { static_cast<int>(array->Elements.size()) });
}

void Compiler::NodeCompile(IndexExpression* index)
{
}

void Compiler::NodeCompile(HashLiteral* hash)
{
	for (auto pair : hash->Elements)
	{
		pair.first->Compile(this);
		if (HasErrors())
		{
			return;
		}
		pair.second->Compile(this);
		if (HasErrors())
		{
			return;
		}
	}
	Emit(OpCode::Constants::OP_HASH, { static_cast<int>(hash->Elements.size()) });
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

void Compiler::SetLastInstruction(const Instructions& instruction)
{
	_previousLastInstruction = _lastInstruction;
	_lastInstruction = instruction;
}
