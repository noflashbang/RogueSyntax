#include <pch.h>


SymbolTable::SymbolTable(std::string scope)
	: _scope(scope), _outer(nullptr)
{
}

SymbolTable::SymbolTable()
	: _scope(SCOPE_GLOBAL), _outer(nullptr)
{
}

SymbolTable::SymbolTable(std::shared_ptr<SymbolTable> outer)
	: _outer(outer), _scope(SCOPE_LOCAL)
{
}

Symbol SymbolTable::Define(const std::string& name)
{
	int index = -1;
	for (int i = _store.size() - 1; i >= 0; i--)
	{
		if (_store[i].Name == name)
		{
			index = i;
		}
	}

	if (index != -1)
	{
		return _store[index];
	}
	
	index = _store.size();
	auto symbol = Symbol{ name, _scope, index };
	_store.push_back(symbol);
	return symbol;	
}

Symbol SymbolTable::Resolve(const std::string& name)
{
	int index = -1;
	for (int i = _store.size() - 1; i >= 0; i--)
	{
		if (_store[i].Name == name)
		{
			index = i;
		}
	}

	if (index == -1 && _outer != nullptr)
	{
		return _outer->Resolve(name);
	}

	if (index == -1)
	{
		throw std::runtime_error("Symbol not found");
	}

	return _store[index];
}

int CompilationUnit::AddInstruction(Instructions instructions)
{
	auto position = UnitInstructions.size();
	UnitInstructions.insert(UnitInstructions.end(), instructions.begin(), instructions.end());

	SetLastInstruction(instructions);
	return position;
}

void CompilationUnit::RemoveLastPop()
{
	if (LastInstructionIs(OpCode::Constants::OP_POP))
	{
		RemoveLastInstruction();
	}
}

bool CompilationUnit::LastInstructionIs(OpCode::Constants opcode)
{
	if (LastInstruction.empty())
	{
		return false;
	}

	return LastInstruction[0] == static_cast<uint8_t>(opcode);
}

void CompilationUnit::RemoveLastInstruction()
{
	UnitInstructions.resize(UnitInstructions.size() - LastInstruction.size());
	SetLastInstruction(PreviousLastInstruction);
}

void CompilationUnit::ChangeOperand(int position, int operand)
{
	auto instruction = OpCode::Make(static_cast<OpCode::Constants>(UnitInstructions[position]), { operand });
	ReplaceInstruction(position, instruction);
}

void CompilationUnit::ReplaceInstruction(int position, Instructions instructions)
{
	for (const auto& instr : instructions)
	{
		UnitInstructions[position++] = instr;
	}
}

void CompilationUnit::SetLastInstruction(const Instructions& instruction)
{
	PreviousLastInstruction = LastInstruction;
	LastInstruction = instruction;
}

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
	if (_CompilationUnits.empty())
	{
		throw std::runtime_error("No compilation units");
	}
	return ByteCode{ _CompilationUnits.top().UnitInstructions, _constants };
}

int Compiler::EnterUnit()
{
	if (_CompilationUnits.empty())
	{
		_CompilationUnits.push(CompilationUnit());
	}
	else
	{
		auto outer = _CompilationUnits.top().SymbolTable;
		_CompilationUnits.push(CompilationUnit(outer));
	}
	return _CompilationUnits.size() - 1;
}

CompilationUnit Compiler::ExitUnit()
{
	auto unit = _CompilationUnits.top();
	_CompilationUnits.pop();
	return unit;
}

int Compiler::AddConstant(std::shared_ptr<IObject> obj)
{
	_constants.push_back(obj);
	return _constants.size() - 1;
}

int Compiler::Emit(OpCode::Constants opcode, std::vector<int> operands)
{
	auto instructions = OpCode::Make(opcode, operands);
	return _CompilationUnits.top().AddInstruction(instructions);
}

void Compiler::NodeCompile(Program* program)
{
	EnterUnit(); // enter global unit
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
	ret->ReturnValue->Compile(this);
	if (HasErrors())
	{
		return;
	}
	Emit(OpCode::Constants::OP_RETURN_VALUE, {});
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
		auto symbol = _CompilationUnits.top().SymbolTable->Define(ident->Value);
		if (symbol.Scope == SCOPE_LOCAL)
		{
			Emit(OpCode::Constants::OP_SET_LOCAL, { symbol.Index });
		}
		else
		{
			Emit(OpCode::Constants::OP_SET_GLOBAL, { symbol.Index });
		}
	}
}

void Compiler::NodeCompile(Identifier* ident)
{
	auto symbol = _CompilationUnits.top().SymbolTable->Resolve(ident->Value);
	if (symbol.Scope == SCOPE_LOCAL)
	{
		Emit(OpCode::Constants::OP_GET_LOCAL, { symbol.Index });
	}
	else
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

	_CompilationUnits.top().RemoveLastPop();
	auto jumpPos = Emit(OpCode::Constants::OP_JUMP, { 9999 });

	auto afterConsequencePos = _CompilationUnits.top().UnitInstructions.size();
	_CompilationUnits.top().ChangeOperand(jumpNotTruthyPos, afterConsequencePos);
	
	if (ifExpr->Alternative != nullptr)
	{
		ifExpr->Alternative->Compile(this);
		if (HasErrors())
		{
			return;
		}
		_CompilationUnits.top().RemoveLastPop();
	}
	else
	{
		Emit(OpCode::Constants::OP_NULL, {});
	}

	auto afterAlternativePos = _CompilationUnits.top().UnitInstructions.size();
	_CompilationUnits.top().ChangeOperand(jumpPos, afterAlternativePos);
}

void Compiler::NodeCompile(FunctionLiteral* function)
{
	EnterUnit();

	for (auto param : function->Parameters)
	{
		auto ident = dynamic_cast<Identifier*>(param.get());
		if (ident == nullptr)
		{
			_errorStack.push(CompilerErrorInfo::New(CompilerError::UnknownError, "Expected identifier"));
			return;
		}
		auto symbol = _CompilationUnits.top().SymbolTable->Define(ident->Value);
	}

	function->Body->Compile(this);
	if (HasErrors())
	{
		return;
	}

	auto unit = ExitUnit();

	if (unit.LastInstructionIs(OpCode::Constants::OP_POP))
	{
		unit.RemoveLastPop();
		unit.AddInstruction(OpCode::Make(OpCode::Constants::OP_RETURN_VALUE, {}));
	}

	if (!unit.LastInstructionIs(OpCode::Constants::OP_RETURN_VALUE))
	{
		unit.AddInstruction(OpCode::Make(OpCode::Constants::OP_RETURN, {}));
	}

	auto index = AddConstant(FunctionCompiledObj::New(unit.UnitInstructions, unit.SymbolTable->NumberOfSymbols(), function->Parameters.size()));
	Emit(OpCode::Constants::OP_CONSTANT, { index });
}

void Compiler::NodeCompile(CallExpression* call)
{
	call->Function->Compile(this);
	if (HasErrors())
	{
		return;
	}

	for (auto arg : call->Arguments)
	{
		arg->Compile(this);
		if (HasErrors())
		{
			return;
		}
	}

	Emit(OpCode::Constants::OP_CALL, { static_cast<int>(call->Arguments.size())});
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
	index->Left->Compile(this);
	if (HasErrors())
	{
		return;
	}
	index->Index->Compile(this);
	if (HasErrors())
	{
		return;
	}
	Emit(OpCode::Constants::OP_INDEX, {});
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


