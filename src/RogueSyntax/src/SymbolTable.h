#pragma once
#include <StandardLib.h>
#include "Decorator.h"



struct Symbol
{
	ScopeType Type;
	std::string Name;
	std::string MangledName;
	int Index;

	uint32_t EncodedIdx();
};

struct SymbolIndex
{
	uint32_t NextSymIndex = 0;
	uint32_t NextFreeIdx = 0;
};

struct ContextMap
{
	SymbolIndex& operator[](const std::string& key)
	{
		if (_contexts.find(key) == _contexts.end())
		{
			_contexts[key] = SymbolIndex();
		}
		return _contexts[key];
	};

private:
	std::map<std::string, SymbolIndex> _contexts;
};

class SymbolTable
{
public:
	SymbolTable() = default;
	~SymbolTable() = default;

	void PushContext(const std::string& context);
	void PopContext();

	Symbol Define(const std::string& name);
	Symbol DefineFunctionName(const std::string& name);
	Symbol DefineExternal(const std::string& name, int idx);
	Symbol Resolve(const std::string& name);
	Symbol DefineFree(const Symbol& symbol);

	inline int NumberOfSymbols() const { return _store.size(); };

private:
	Decorator _decorator;
	ContextMap _contexts;
	std::vector<Symbol> _store;
};