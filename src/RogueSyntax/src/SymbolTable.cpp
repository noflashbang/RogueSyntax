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

void SymbolTable::PushContext(const std::string& context)
{
	_decorator.Push(context);
}

void SymbolTable::PopContext()
{
	_decorator.Pop();
}

Symbol SymbolTable::Define(const std::string& name)
{
	auto decorated = _decorator.DecorateWithCurrentContex(name);
	auto context = _decorator.DecoratedContext();
	auto& idxMap = _contexts[context];
	int index = -1;
	for (int i = _store.size() - 1; i >= 0; i--)
	{
		if (_store[i].MangledName == decorated)
		{
			index = i;
		}
	}

	if (index != -1)
	{
		return _store[index];
	}

	index = idxMap.NextSymIndex++;
	auto type = context.empty() ? ScopeType::SCOPE_GLOBAL : ScopeType::SCOPE_LOCAL;
	auto symbol = Symbol{ type, name, decorated, index };
	_store.push_back(symbol);
	return symbol;
}

Symbol SymbolTable::DefineFunctionName(const std::string& name)
{
	auto decorated = _decorator.DecorateWithCurrentContex(name);
	int index = 0; //not used
	auto symbol = Symbol{ ScopeType::SCOPE_FUNCTION, name, decorated, index};
	_store.push_back(symbol);
	return symbol;
}

Symbol SymbolTable::DefineExternal(const std::string& name, int idx)
{
	auto decorated = _decorator.DecorateExternal(name);
	auto symbol = Symbol{ ScopeType::SCOPE_EXTERN, name, decorated, idx  };
	_store.push_back(symbol);
	return symbol;
}

Symbol SymbolTable::Resolve(const std::string& name)
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

	if (index == -1)
	{
		auto allNames = _decorator.DecorateWithAllContexts(name);
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

		if (index == -1)
		{
			throw std::runtime_error("Symbol not found");
		}

		auto upval = _store[index];
		if (upval.Type == ScopeType::SCOPE_GLOBAL || upval.Type == ScopeType::SCOPE_EXTERN)
		{
			return upval;
		}

		return DefineFree(upval);
	}

	return _store[index];
}

Symbol SymbolTable::DefineFree(const Symbol& symbol)
{
	auto context = _decorator.DecoratedContext();
	auto& idxMap = _contexts[context];
	int index = idxMap.NextFreeIdx++;
	auto freeSym = Symbol{ ScopeType::SCOPE_FREE, symbol.Name, symbol.MangledName, index};
	_store.push_back(freeSym);
	return freeSym;
}

