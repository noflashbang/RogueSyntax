#pragma once

#include "StandardLib.h"
#include "Identifiable.h"
#include "IObject.h"

class ObjectStore
{
public:
	ObjectStore();
	~ObjectStore();

	void Add(const std::shared_ptr<IObject>& obj);
	std::shared_ptr<ObjectFactory> Factory() { return std::make_shared<ObjectFactory>(this); }

private:
	std::vector<std::shared_ptr<IObject>> _store;
};

class ObjectFactory
{
public:
	ObjectFactory(ObjectStore* store);
	~ObjectFactory();

	template <typename T, typename... Args>
	T* New(Args... args) const
	{
		static_assert(std::is_base_of<IObject, T>::value, "T must derive from IObject");

		auto obj = std::make_shared<T>(args...);
		_store->Add(obj);
		return obj.get();
	}

	template <typename T>
	IObject* Clone(const T* obj)
	{
		static_assert(std::is_base_of<IObject, T>::value, "T must derive from IObject");

		auto clone = obj->Clone(this);
		std::shared_ptr<IObject> ptr(clone);
		_store->Add(ptr);
		return clone;
	}

private:
	ObjectStore* _store;
};
