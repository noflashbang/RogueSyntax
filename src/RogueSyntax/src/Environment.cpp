#include "Environment.h"
#include "Environment.h"
#include "Environment.h"
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

void ScopedEnvironment::Set(const std::string& name, const IObject* value)
{
	auto it = IdentifierStore.find(name);
	if (it == IdentifierStore.end() && Parent != nullptr)
	{
		Parent->Set(name, value);
	}
	else
	{
		IdentifierStore[name] = value;
	}
}

const IObject* ScopedEnvironment::Get(const std::string& name) const
{
	auto it = IdentifierStore.find(name);
	if (it != IdentifierStore.end())
	{
		return it->second;
	}

	if (Parent != nullptr)
	{
		return Parent->Get(name);
	}

	return nullptr;
}

void ScopedEnvironment::Update(const size_t id, const IObject* updatedValue)
{
	for (auto& [key, value] : IdentifierStore)
	{
		if (value->Id() == id)
		{
			IdentifierStore[key] = updatedValue;
			return;
		}
	}
	if (Parent != nullptr)
	{
		return Parent->Update(id, updatedValue);
	}
}

Environment::Environment() : _current(0,0)
{
}

Environment::~Environment()
{
}

void Environment::Set(const uint32_t env, const std::string& name, const IObject* value)
{
	EnvironmentHandle envHandle(env);
	auto envObj = _environments[envHandle._internal.Index];
	if (envObj->Handle.Handle != envHandle.Handle)
	{
		throw std::runtime_error("Invalid environment");
	}
	envObj->Set(name, value);
}

const IObject* Environment::Get(const uint32_t env, const std::string& name) const
{
	EnvironmentHandle envHandle(env);
	auto envObj = _environments[envHandle._internal.Index];
	if (envObj->Handle.Handle != envHandle.Handle)
	{
		throw std::runtime_error("Invalid environment");
	}
	return envObj->Get(name);
}

void Environment::Update(const uint32_t env, const size_t id, const IObject* updatedValue)
{
	EnvironmentHandle envHandle(env);
	auto envObj = _environments[envHandle._internal.Index];
	if (envObj->Handle.Handle != envHandle.Handle)
	{
		throw std::runtime_error("Invalid environment");
	}
	envObj->Update(id, updatedValue);
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

