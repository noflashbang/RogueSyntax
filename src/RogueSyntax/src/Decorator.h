#pragma once
#include <StandardLib.h>

struct Decorator
{
	void Push(const std::string& context) { ContextStack.push(context); };
	void Pop() { ContextStack.pop(); };

	std::string DecorateExternal(const std::string& name);
	std::string DecoratedContext();
	std::string DecorateWithCurrentContex(const std::string& name);
	std::vector<std::string> DecorateWithAllContexts(const std::string& name);

	std::stack<std::string> ContextStack;

private:
	std::string Decorate(const std::string& name, std::stack<std::string> stack);
	std::string GetDecoratedFromStack(std::stack<std::string> stack);
	std::string Formater(const std::string& left, const std::string& right);
};

