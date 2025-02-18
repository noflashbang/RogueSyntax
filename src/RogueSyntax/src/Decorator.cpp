#include "Decorator.h"
#include <pch.h>

std::string Decorator::DecorateExternal(const std::string& name)
{
	return Formater("_extern", name);
}
std::string Decorator::DecoratedContext()
{
	return GetDecoratedFromStack(ContextStack);
}
std::string Decorator::DecorateWithCurrentContex(const std::string& name)
{
	return Formater(DecoratedContext(), name);
}
std::vector<std::string> Decorator::DecorateWithAllContexts(const std::string& name)
{
	std::vector<std::string> result;
	auto stack = ContextStack;
	while (!stack.empty())
	{
		result.push_back(Formater(GetDecoratedFromStack(stack), name));
		stack.pop();
	}

	result.push_back(DecorateExternal(name));
	return result;
}

std::string Decorator::Decorate(const std::string& name, std::stack<std::string> stack)
{
	auto result = GetDecoratedFromStack(stack);
	return Formater(result, name);
}
std::string Decorator::GetDecoratedFromStack(std::stack<std::string> stack)
{
	std::string result = stack.top();
	stack.pop();
	while (!stack.empty())
	{
		auto top = stack.top();
		stack.pop();
		result = Formater(top, result);
	}
	return result;
}
std::string Decorator::Formater(const std::string& left, const std::string& right)
{
	return std::format("{}::{}", left, right);
}