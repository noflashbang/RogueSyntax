#include "Environment.h"

Environment::Environment()
{
	Outer = nullptr;
}

Environment::Environment(const std::shared_ptr<Environment>& outer) : Outer(outer)
{
}

void Environment::Set(const std::string& name, const std::shared_ptr<IObject>& value)
{
	Store[name] = value;
}

std::shared_ptr<IObject> Environment::Get(const std::string& name)
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

std::shared_ptr<Environment> Environment::New()
{
	return std::make_shared<Environment>();
}

std::shared_ptr<Environment> Environment::NewEnclosed(const std::shared_ptr<Environment>& outer)
{
	return std::make_shared<Environment>(outer);
}