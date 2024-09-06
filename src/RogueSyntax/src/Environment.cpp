#include "Environment.h"

Environment::Environment()
{
	Outer = nullptr;
}

Environment::Environment(Environment* outer)
{
	Outer = outer;
}

void Environment::Set(const std::string& name, IObject* value)
{
	Store[name] = value;
}

IObject* Environment::Get(const std::string& name)
{
	auto it = Store.find(name);
	if (it != Store.end())
	{
		return it->second;
	}

	if (Outer != nullptr)
	{
		return Outer->Get(name);
	}

	return nullptr;
}