#pragma once
#include <StandardLib.h>
#include "Decorator.h"

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

	void PushScopeContext(const std::string& context);
	void PopScopeContext();
	std::string CurrentScopeContext();

	void PushStackContext();
	void PopStackContext();
	uint32_t CurrentStackContext();

	Symbol Define(const std::string& name);
	Symbol DefineFunctionName(const std::string& name);
	Symbol DefineExternal(const std::string& name, int idx);
	Symbol Resolve(const std::string& name);
	Symbol DefineFree(const Symbol& symbol);

	uint32_t NumberOfSymbolsInContext(uint32_t stackContext);
	std::vector<Symbol> SymbolsInContext(uint32_t stackContext);
	std::vector<Symbol> FreeSymbolsInContext(uint32_t stackContext);

	std::vector<Symbol> GetSymbols();

private:
	int FindInCurrentContextName(const std::string& name);
	int FindInAllContexts(const std::string& name);

	Decorator _decorator;
	ContextMap _contexts;
	std::vector<Symbol> _store;
	std::vector<Symbol> _free;

	std::stack<uint32_t> _stack;
	uint32_t _stackIndex = 0;
};