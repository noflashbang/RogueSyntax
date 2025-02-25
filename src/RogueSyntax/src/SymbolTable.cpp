#include "SymbolTable.h"
#include <pch.h>

uint32_t Symbol::EncodedIdx()
{
	if (Type == ScopeType::SCOPE_GLOBAL)
	{
		return (Index & 0x3FFF);
	}
	else if (Type == ScopeType::SCOPE_LOCAL)
	{
		return ((Index & 0x3FFF)) | 0x8000;
	}
	else if (Type == ScopeType::SCOPE_EXTERN)
	{
		return ((Index & 0x3FFF)) | 0x4000;
	}
	else if (Type == ScopeType::SCOPE_FREE)
	{
		return ((Index & 0x3FFF)) | 0xC000;
	}
	else
	{
		throw std::runtime_error("Invalid symbol scope");
	}
}

void SymbolTable::PushScopeContext(const std::string& context)
{
	_decorator.Push(context);
}

void SymbolTable::PopScopeContext()
{
	_decorator.Pop();
}

std::string SymbolTable::CurrentScopeContext()
{
	return _decorator.DecoratedContext();
}

void SymbolTable::PushStackContext()
{
	_stackIndex++;
	_stack.push(_stackIndex);
}

void SymbolTable::PopStackContext()
{
	_stack.pop();
}

uint32_t SymbolTable::CurrentStackContext()
{
	return _stack.top();
}


Symbol SymbolTable::Define(const std::string& name)
{
	auto decorated = _decorator.DecorateWithCurrentContex(name);
	auto context = _decorator.DecoratedContext();
	auto& idxMap = _contexts[context];
	int index = -1;
	index = FindInCurrentContextName(name);
	if (index != -1)
	{
		return _store[index];
	}

	index = FindInAllContexts(name);
	if (index != -1)
	{
		return _store[index];
	}
	index = idxMap.NextSymIndex++;
	auto type = _stack.size() <= 1 ? ScopeType::SCOPE_GLOBAL : ScopeType::SCOPE_LOCAL;
	auto symbol = Symbol{ type, name, decorated, context, _stack.top(), index };
	_store.push_back(symbol);
	return symbol;
}

Symbol SymbolTable::DefineFunctionName(const std::string& name)
{
	auto context = _decorator.DecoratedContext();
	auto decorated = _decorator.DecorateWithCurrentContex(name);
	int index = 0; //not used
	auto symbol = Symbol{ ScopeType::SCOPE_FUNCTION, name, decorated, context, _stack.top(), index};
	_store.push_back(symbol);
	return symbol;
}

Symbol SymbolTable::DefineExternal(const std::string& name, int idx)
{
	auto decorated = _decorator.DecorateExternal(name);
	auto symbol = Symbol{ ScopeType::SCOPE_EXTERN, name, decorated, "EXTERN", 0, idx};
	_store.push_back(symbol);
	return symbol;
}

Symbol SymbolTable::Resolve(const std::string& name)
{
	auto decorated = _decorator.DecorateWithCurrentContex(name);
	int index = -1;
	index = FindInCurrentContextName(name);
	if (index != -1)
	{
		return _store[index];
	}

	index = FindInAllContexts(name);
	if (index != -1)
	{
		auto upval = _store[index];
		if (upval.Type == ScopeType::SCOPE_GLOBAL || upval.Type == ScopeType::SCOPE_EXTERN || upval.stackContext == _stack.top())
		{
			return upval;
		}

		return DefineFree(upval);
		return _store[index];
	}
	throw std::runtime_error("Symbol not found");
}

Symbol SymbolTable::DefineFree(const Symbol& symbol)
{
	auto symCpy = symbol;
	symCpy.stackContext = _stack.top();
	_free.push_back(symCpy);

	auto context = _decorator.DecoratedContext();
	auto& idxMap = _contexts[context];
	int index = idxMap.NextFreeIdx++;
	auto freeSym = Symbol{ ScopeType::SCOPE_FREE, symbol.Name, symbol.MangledName, context, _stack.top(), index};
	_store.push_back(freeSym);
	return freeSym;
}

uint32_t SymbolTable::NumberOfSymbolsInContext(uint32_t stackContext)
{
	uint32_t cnt = 0;
	for (auto& it : _store)
	{
		if (it.stackContext == stackContext && it.Type != ScopeType::SCOPE_FUNCTION)
		{
			cnt++;
		}
	}
	return cnt;
}

std::vector<Symbol> SymbolTable::SymbolsInContext(uint32_t stackContext)
{
	std::vector<Symbol> symbols;
	for (auto& it : _store)
	{
		if (it.stackContext == stackContext)
		{
			symbols.push_back(it);
		}
	}
	return symbols;
}

std::vector<Symbol> SymbolTable::FreeSymbolsInContext(uint32_t stackContext)
{
	std::vector<Symbol> symbols;
	for (auto& it : _free)
	{
		if (it.stackContext == stackContext)
		{
			symbols.push_back(it);
		}
	}
	return symbols;
}

std::vector<Symbol> SymbolTable::GetSymbols()
{
	return _store;
}

int SymbolTable::FindInCurrentContextName(const std::string& name)
{
	auto decorated = _decorator.DecorateWithCurrentContex(name);
	int index = -1;
	for (int i = _store.size() - 1; i >= 0; i--)
	{
		if (_store[i].MangledName == decorated)
		{
			index = i;
		}
	}
	return index;
}

int SymbolTable::FindInAllContexts(const std::string& name)
{
	auto allNames = _decorator.DecorateWithAllContexts(name);
	int index = -1;
	for (auto& name : allNames)
	{
		for (int i = _store.size() - 1; i >= 0; i--)
		{
			if (_store[i].MangledName == name)
			{
				index = i;
			}
		}
		if (index != -1)
		{
			break;
		}
	}
	return index;
}



