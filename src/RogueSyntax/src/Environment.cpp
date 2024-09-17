#include "Environment.h"
#include "pch.h"

ScopedEnvironment::ScopedEnvironment(EnvironmentHandle hnd) : Handle(hnd)
{
	Parent = nullptr;
}

void ScopedEnvironment::SetParent(const std::shared_ptr<ScopedEnvironment>& parent)
{
	Parent = parent;
}

ScopedEnvironment::~ScopedEnvironment()
{
}

void ScopedEnvironment::Set(const std::string& name, const std::shared_ptr<IObject>& value)
{
	auto it = Store.find(name);
	if (it == Store.end() && Parent != nullptr)
	{
		Parent->Set(name, value);
	}
	else
	{
		Store[name] = value;
	}
}

std::shared_ptr<IObject> ScopedEnvironment::Get(const std::string& name) const
{
	auto it = Store.find(name);
	if (it != Store.end())
	{
		return it->second;
	}

	if (Parent != nullptr)
	{
		return Parent->Get(name);
	}

	return nullptr;
}


Environment::Environment() : _current(0,0)
{
}

Environment::~Environment()
{
}

void Environment::Set(const uint32_t env, const std::string& name, const std::shared_ptr<IObject>& value)
{
	EnvironmentHandle envHandle(env);
	auto envObj = _environments[envHandle._internal.Index];
	if (envObj->Handle.Handle != envHandle.Handle)
	{
		throw std::runtime_error("Invalid environment");
	}
	envObj->Set(name, value);
}

std::shared_ptr<IObject> Environment::Get(const uint32_t env, const std::string& name) const
{
	EnvironmentHandle envHandle(env);
	auto envObj = _environments[envHandle._internal.Index];
	if (envObj->Handle.Handle != envHandle.Handle)
	{
		throw std::runtime_error("Invalid environment");
	}
	return envObj->Get(name);
}

uint32_t Environment::New()
{
	auto holder = Allocate();
	return holder.Handle;
}

uint32_t Environment::NewEnclosed(const uint32_t outer)
{
	EnvironmentHandle envHandle(outer);
	auto envObj = _environments[envHandle._internal.Index];

	if (envObj->Handle.Handle != envHandle.Handle)
	{
		throw std::runtime_error("Invalid environment");
	}
	auto holder = Allocate(envHandle);
	return holder.Handle;
}

void Environment::Release(const uint32_t env)
{
	Free(env);
}

EnvironmentHandle Environment::Allocate()
{
	if (_environments.size() >= 0xFFFFFF)
	{
		throw std::runtime_error("Environment limit reached");
	}

	if (_freeList.empty())
	{
		auto idx = _environments.size();
		auto gen = 0;
		EnvironmentHandle env(idx, gen);
		_environments.push_back(std::make_shared<ScopedEnvironment>(env));
		return env;
	}
	else
	{
		auto oldEnv = _freeList.back();
		_freeList.pop_back();
		if (oldEnv._internal.Generation == 255)
		{
			return Allocate();
		}
		EnvironmentHandle env(oldEnv._internal.Index, oldEnv._internal.Generation + 1);
		_environments[env._internal.Index] = std::make_shared<ScopedEnvironment>(env);
		return env;
	}
}

EnvironmentHandle Environment::Allocate(const EnvironmentHandle parent)
{
	auto child = Allocate();
	auto parentEnv = _environments[parent._internal.Index];
	auto childEnv = _environments[child._internal.Index];
	childEnv->SetParent(parentEnv);
	return child;
}

void Environment::Free(const EnvironmentHandle env)
{
	_freeList.push_back(env);
	_environments[env._internal.Index] = nullptr;
}

