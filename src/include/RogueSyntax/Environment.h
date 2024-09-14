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

	void Set(const std::string& name, const std::shared_ptr<IObject>& value);
	std::shared_ptr<IObject> Get(const std::string& name) const;

	EnvironmentHandle Handle;
	std::unordered_map<std::string, std::shared_ptr<IObject>> Store;
	std::shared_ptr<ScopedEnvironment> Parent;
};

class Environment
{
public:
	Environment();
	~Environment();

	void Set(const uint32_t env, const std::string& name, const std::shared_ptr<IObject>& value);
	std::shared_ptr<IObject> Get(const uint32_t env, const std::string& name) const;

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