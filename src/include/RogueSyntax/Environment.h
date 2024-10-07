#pragma once
#include "StandardLib.h"

struct EnvironmentHandle
{
	union
	{
		const struct
		{
			uint32_t Index : 24;
			uint32_t Generation : 8;
		} _internal;
		uint32_t Handle;
	};
	
	EnvironmentHandle(uint32_t index, uint32_t generation) : _internal{ index, generation }
	{
	};
	EnvironmentHandle(uint32_t handle) : Handle(handle)
	{
	};
};

struct ScopedEnvironment
{
	ScopedEnvironment(EnvironmentHandle hnd);
	~ScopedEnvironment();

	void SetParent(const std::shared_ptr<ScopedEnvironment>& env);

	void Set(const std::string& name, const IObject* value);
	const IObject* Get(const std::string& name) const;

	//Used to update the value of an object in the environment, since the object is immutable
	void Update(const size_t id, const IObject* updatedValue);

	EnvironmentHandle Handle;
	std::unordered_map<std::string, const IObject*> IdentifierStore;
	std::shared_ptr<ScopedEnvironment> Parent;
};

class Environment
{
public:
	Environment();
	~Environment();

	void Set(const uint32_t env, const std::string& name, const IObject* value);
	const IObject* Get(const uint32_t env, const std::string& name) const;

	void Update(const uint32_t env, const size_t id, const IObject* updatedValue);

	uint32_t New();
	uint32_t NewEnclosed(const uint32_t parent);
	void Release(const uint32_t env);

private:
	EnvironmentHandle Allocate();
	EnvironmentHandle Allocate(const EnvironmentHandle parent);
	void Free(const EnvironmentHandle env);

	EnvironmentHandle _current;
	std::vector<std::shared_ptr<ScopedEnvironment>> _environments;
	std::vector<EnvironmentHandle> _freeList;
};