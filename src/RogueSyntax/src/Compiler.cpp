#include "Compiler.h"
#include <pch.h>

Compiler::Compiler(const std::shared_ptr<ObjectFactory> factory) : _factory(factory)
{
}

Compiler::~Compiler()
{
}

ObjectCode Compiler::Compile(const std::shared_ptr<Program>&program, const std::shared_ptr<BuiltIn>& externs, const std::string& unitName)
{
	_constants.clear();
	_constants.reserve(128);
	_externals = externs;

	auto& builtins = _externals->GetBuiltInNames();
	for (auto& name : builtins)
	{
		_symbolTable.DefineExternal(name, _externals->BuiltInIdx(name));
	}

	Compile(program.get());
	return ObjectCode{ _CompilationUnits.top().UnitInstructions, _symbolTable.GetSymbols()};
}

CompilerError Compiler::Compile(INode* node)
{
	node->Compile(this);
	if (!_errorStack.empty())
	{
		return _errorStack.top().Error;
	}
	return CompilerError::NoError;
}

int Compiler::EnterUnit(const std::string& context)
{
	_symbolTable.PushStackContext();
	EnterScope(context);
	_CompilationUnits.push(CompilationUnit());	
	return _CompilationUnits.size() - 1;
}

CompilationUnit Compiler::ExitUnit()
{
	_symbolTable.PopStackContext();
	ExitScope();
	auto unit = _CompilationUnits.top();
	_CompilationUnits.pop();
	return unit;
}

void Compiler::EnterScope(const std::string& scope)
{
	_symbolTable.PushScopeContext(scope);
}
void Compiler::ExitScope()
{
	_symbolTable.PopScopeContext();
}

uint32_t Compiler::AddConstant(const IObject* obj)
{
	_constants.push_back(obj);
	return _constants.size() - 1;
}

int Compiler::Emit(OpCode::Constants opcode, std::vector<uint32_t> operands)
{
	auto instructions = OpCode::Make(opcode, operands);
	return _CompilationUnits.top().AddInstruction(instructions);
}

int Compiler::Emit(OpCode::Constants opcode, std::vector<uint32_t> operands, RSInstructions data)
{
	auto instructions = OpCode::Make(opcode, operands);
	instructions.insert(instructions.end(), data.begin(), data.end());
	return _CompilationUnits.top().AddInstruction(instructions);
}

int Compiler::EmitGet(Symbol symbol)
{
	if (symbol.Type == ScopeType::SCOPE_FUNCTION)
	{
		return Emit(OpCode::Constants::OP_CUR_CLOSURE, {});
	}
	else
	{
		return Emit(OpCode::Constants::OP_GET, { symbol.EncodedIdx() });
	}
}

int Compiler::EmitSet(Symbol symbol)
{
	if (symbol.Type == ScopeType::SCOPE_FUNCTION)
	{
		return Emit(OpCode::Constants::OP_CUR_CLOSURE, {});
	}
	else
	{
		return Emit(OpCode::Constants::OP_SET, { symbol.EncodedIdx() });
	}
}

void Compiler::NodeCompile(const Program* program, const std::string& unitName)
{
	EnterUnit(unitName); // enter global unit
	for (auto& stmt : program->Statements)
	{
		stmt->Compile(this);
		if (HasErrors())
		{
			return;
		}
	}
}

void Compiler::NodeCompile(const BlockStatement* block)
{
	int ln = block->BaseToken.Location.Line;
	//EnterScope(std::format("BLOCK@{}",ln) ); // enter block unit
	for (auto& stmt : block->Statements)
	{
		stmt->Compile(this);
		if (HasErrors())
		{
			return;
		}
	}
	//ExitScope();
}

void Compiler::NodeCompile(const ExpressionStatement* expression)
{

	expression->Expression->Compile(this);
	if (HasErrors())
	{
		return;
	}
	Emit(OpCode::Constants::OP_POP, {});
}

void Compiler::NodeCompile(const ReturnStatement* ret)
{
	ret->ReturnValue->Compile(this);
	if (HasErrors())
	{
		return;
	}
	Emit(OpCode::Constants::OP_RET_VAL, {});
}

void Compiler::NodeCompile(const LetStatement* let)
{
	if (let->Name->IsThisA<Identifier>())
	{
		auto* ident = dynamic_cast<const Identifier*>(let->Name);
		auto symbol = _symbolTable.Define(ident->Value);
		let->Value->Compile(this);
		if (HasErrors())
		{
			return;
		}

		EmitSet(symbol);
	}
	else if (let->Name->IsThisA<IndexExpression>())
	{
		auto* index = dynamic_cast<const IndexExpression*>(let->Name);
		if (!index->Left->IsThisA<Identifier>())
		{
			_errorStack.push(CompilerErrorInfo::New(CompilerError::UnknownError, "Expected identifier"));
			return;
		}
		auto* ident = dynamic_cast<const Identifier*>(index->Left);
		auto symbol = _symbolTable.Resolve(ident->Value);

		//get the lvalue
		index->Compile(this);
		if (HasErrors())
		{
			return;
		}

		//remove the last index instruction
		_CompilationUnits.top().RemoveLastInstruction();

		//get the rvalue
		let->Value->Compile(this);
		if (HasErrors())
		{
			return;
		}

		Emit(OpCode::Constants::OP_SET_ASSIGN, { symbol.EncodedIdx() });
	}
	else
	{
		_errorStack.push(CompilerErrorInfo::New(CompilerError::UnknownError, "Unknown let statement"));
	}
}

void Compiler::NodeCompile(const Identifier* ident)
{
	auto sym = _symbolTable.Resolve(ident->Value);
	EmitGet(sym);
}

void Compiler::NodeCompile(const IntegerLiteral* integer)
{
	//auto obj = _factory->New<IntegerObj>(integer->Value);
	//auto index = AddConstant(obj);
	//Emit(OpCode::Constants::OP_CONSTANT, { index });
	int intVal = integer->Value;
	uint32_t val = reinterpret_cast<uint32_t&>(intVal);
	Emit(OpCode::Constants::OP_LINT, { val });
}

void Compiler::NodeCompile(const BooleanLiteral* boolean)
{
	boolean->Value ? Emit(OpCode::Constants::OP_TRUE, {}) : Emit(OpCode::Constants::OP_FALSE, {});
}

void Compiler::NodeCompile(const StringLiteral* string)
{
	//auto obj = _factory->New<StringObj>(string->Value);
	//auto index = AddConstant(obj);
	//Emit(OpCode::Constants::OP_CONSTANT, { index });
	std::string str = string->Value;
	uint32_t len = str.size();
	RSInstructions data;
	for (auto c : str)
	{
		data.push_back(c);
	}
	Emit(OpCode::Constants::OP_LSTRING, { len }, data);
}

void Compiler::NodeCompile(const DecimalLiteral* decimal)
{
	//auto obj = _factory->New<DecimalObj>(decimal->Value);
	//auto index = AddConstant(obj);
	//Emit(OpCode::Constants::OP_CONSTANT, { index });

	float decVal = decimal->Value;
	uint32_t val = reinterpret_cast<uint32_t&>(decVal);
	Emit(OpCode::Constants::OP_LDECIMAL, { val });
}

void Compiler::NodeCompile(const PrefixExpression* prefix)
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

void Compiler::NodeCompile(const InfixExpression* infix)
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
		Emit(OpCode::Constants::OP_EQ, {});
	}
	else if (infix->Operator == "!=")
	{
		Emit(OpCode::Constants::OP_NEQ, {});
	}
	else if (infix->Operator == ">")
	{
		Emit(OpCode::Constants::OP_GT, {});
	}
	else if (infix->Operator == ">=")
	{
		Emit(OpCode::Constants::OP_GTE, {});
	}
	else if (infix->Operator == "<")
	{
		Emit(OpCode::Constants::OP_LT, {});
	}
	else if (infix->Operator == "<=")
	{
		Emit(OpCode::Constants::OP_LTE, {});
	}
	else if (infix->Operator == "&&")
	{
		Emit(OpCode::Constants::OP_AND, {});
	}
	else if (infix->Operator == "||")
	{
		Emit(OpCode::Constants::OP_OR, {});
	}
	else
	{
		_errorStack.push(CompilerErrorInfo::New(CompilerError::UnknownOperator, std::format("Unknown operator {}", infix->Operator)));
	}
}

void Compiler::NodeCompile(const IfStatement* ifExpr)
{
	ifExpr->Condition->Compile(this);
	if (HasErrors())
	{
		return;
	}

	auto jumpNotTruthyPos = Emit(OpCode::Constants::OP_JUMPIFZ, { 9999 });
	//
	ifExpr->Consequence->Compile(this);
	if (HasErrors())
	{
		return;
	}

	if (ifExpr->Alternative != nullptr)
	{
		auto jumpPos = Emit(OpCode::Constants::OP_JUMP, { 9999 });
		auto afterConsequencePos = _CompilationUnits.top().UnitInstructions.size();
		_CompilationUnits.top().ChangeOperand(jumpNotTruthyPos, afterConsequencePos);

		ifExpr->Alternative->Compile(this);
		if (HasErrors())
		{
			return;
		}
		auto afterAlternativePos = _CompilationUnits.top().UnitInstructions.size();
		_CompilationUnits.top().ChangeOperand(jumpPos, afterAlternativePos);
	}
	else
	{
		auto afterConsequencePos = _CompilationUnits.top().UnitInstructions.size();
		_CompilationUnits.top().ChangeOperand(jumpNotTruthyPos, afterConsequencePos);
	}
}

void Compiler::NodeCompile(const FunctionLiteral* function)
{
	EnterUnit(function->Name);

	auto stackContext = _symbolTable.CurrentStackContext();

	if (!function->Name.empty())
	{
		auto fnSymbol = _symbolTable.DefineFunctionName(function->Name);
	}

	for (auto* param : function->Parameters)
	{
		auto* ident = dynamic_cast<Identifier*>(param);
		if (ident == nullptr)
		{
			_errorStack.push(CompilerErrorInfo::New(CompilerError::UnknownError, "Expected identifier"));
			return;
		}
		auto symbol = _symbolTable.Define(ident->Value);
	}

	function->Body->Compile(this);
	if (HasErrors())
	{
		return;
	}
	auto unit = ExitUnit();

	auto frees = _symbolTable.FreeSymbolsInContext(stackContext);
	for (auto& sym : frees)
	{
		EmitGet(sym);
	}
	
	if (unit.LastInstructionIs(OpCode::Constants::OP_POP))
	{
		unit.RemoveLastPop();
		unit.AddInstruction(OpCode::Make(OpCode::Constants::OP_RET_VAL, {}));
	}

	if (!unit.LastInstructionIs(OpCode::Constants::OP_RET_VAL))
	{
		unit.AddInstruction(OpCode::Make(OpCode::Constants::OP_RETURN, {}));
	}
	//auto obj = _factory->New<FunctionCompiledObj>(unit.UnitInstructions, _symbolTable.NumberOfSymbolsInContext(stackContext), static_cast<int>(function->Parameters.size()));
	//auto index = AddConstant(obj);

	Emit(OpCode::Constants::OP_LFUN, { static_cast<uint32_t>(_symbolTable.NumberOfSymbolsInContext(stackContext)), static_cast<uint32_t>(function->Parameters.size()), static_cast<uint32_t>(unit.UnitInstructions.size()) }, unit.UnitInstructions);
	Emit(OpCode::Constants::OP_CLOSURE, { static_cast<uint32_t>(frees.size())});
}

void Compiler::NodeCompile(const CallExpression* call)
{
	call->Function->Compile(this);
	if (HasErrors())
	{
		return;
	}

	for (auto& arg : call->Arguments)
	{
		arg->Compile(this);
		if (HasErrors())
		{
			return;
		}
	}

	Emit(OpCode::Constants::OP_CALL, { static_cast<uint32_t>(call->Arguments.size())});
}

void Compiler::NodeCompile(const ArrayLiteral* array)
{
	for(auto& elem : array->Elements)
	{
		elem->Compile(this);
		if (HasErrors())
		{
			return;
		}
	}
	Emit(OpCode::Constants::OP_ARRAY, { static_cast<uint32_t>(array->Elements.size()) });
}

void Compiler::NodeCompile(const IndexExpression* index)
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

void Compiler::NodeCompile(const HashLiteral* hash)
{
	for (auto& pair : hash->Elements)
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
	Emit(OpCode::Constants::OP_HASH, { static_cast<uint32_t>(hash->Elements.size()) });
}

void Compiler::NodeCompile(const NullLiteral* null)
{
	Emit(OpCode::Constants::OP_NULL, {});
}

void Compiler::NodeCompile(const WhileStatement* whileExp)
{
	auto conditionPos = _CompilationUnits.top().UnitInstructions.size();
	whileExp->Condition->Compile(this);
	if (HasErrors())
	{
		return;
	}

	auto jumpNotTruthyPos = Emit(OpCode::Constants::OP_JUMPIFZ, { 9999 });
	//
	whileExp->Action->Compile(this);
	if (HasErrors())
	{
		return;
	}

	auto jumpPos = Emit(OpCode::Constants::OP_JUMP, { static_cast<uint32_t>(conditionPos) });
	auto afterBodyPos = _CompilationUnits.top().UnitInstructions.size();
	_CompilationUnits.top().ChangeOperand(jumpNotTruthyPos, afterBodyPos);

	//check for break and continue
	while (!_CompilationUnits.top().LoopJumps.empty())
	{
		auto jump = _CompilationUnits.top().LoopJumps.top();
		_CompilationUnits.top().LoopJumps.pop();
		if (jump.Type == LoopJumpType::LOOP_JUMP_CONTINUE)
		{
			_CompilationUnits.top().ChangeOperand(jump.Instruction, conditionPos);
		}
		
		if (jump.Type == LoopJumpType::LOOP_JUMP_BREAK)
		{
			_CompilationUnits.top().ChangeOperand(jump.Instruction, afterBodyPos);
		}
	}
}

void Compiler::NodeCompile(const ForStatement* forExp)
{
	forExp->Init->Compile(this);
	if (HasErrors())
	{
		return;
	}

	auto conditionPos = _CompilationUnits.top().UnitInstructions.size();
	forExp->Condition->Compile(this);
	if (HasErrors())
	{
		return;
	}

	auto jumpNotTruthyPos = Emit(OpCode::Constants::OP_JUMPIFZ, { 9999 });
	forExp->Action->Compile(this);
	if (HasErrors())
	{
		return;
	}

	auto afterBodyPos = _CompilationUnits.top().UnitInstructions.size();

	forExp->Post->Compile(this);
	if (HasErrors())
	{
		return;
	}
	auto jumpPos = Emit(OpCode::Constants::OP_JUMP, { static_cast<uint32_t>(conditionPos) });
	
	auto afterLoopPos = _CompilationUnits.top().UnitInstructions.size();

	_CompilationUnits.top().ChangeOperand(jumpNotTruthyPos, afterLoopPos);

	//check for break and continue
	while (!_CompilationUnits.top().LoopJumps.empty())
	{
		auto jump = _CompilationUnits.top().LoopJumps.top();
		_CompilationUnits.top().LoopJumps.pop();
		if (jump.Type == LoopJumpType::LOOP_JUMP_CONTINUE)
		{
			_CompilationUnits.top().ChangeOperand(jump.Instruction, afterBodyPos);
		}

		if (jump.Type == LoopJumpType::LOOP_JUMP_BREAK)
		{
			_CompilationUnits.top().ChangeOperand(jump.Instruction, afterLoopPos);
		}
	}
}

void Compiler::NodeCompile(const ContinueStatement* cont)
{
	auto location = Emit(OpCode::Constants::OP_JUMP, { 9999 });
	_CompilationUnits.top().LoopJumps.push({LoopJumpType::LOOP_JUMP_CONTINUE, location});
}

void Compiler::NodeCompile(const BreakStatement* brk)
{
	auto location = Emit(OpCode::Constants::OP_JUMP, { 9999 });
	_CompilationUnits.top().LoopJumps.push({ LoopJumpType::LOOP_JUMP_BREAK, location });
}
