#include "pch.h"

ObjectStore::ObjectStore()
{
	_store.reserve(500);
}

ObjectStore::~ObjectStore()
{
}

void ObjectStore::Add(const std::shared_ptr<IObject>& obj)
{
	_store.emplace_back(std::move(obj));
}


ObjectFactory::ObjectFactory(ObjectStore* store)
	: _store(store)
{
}

ObjectFactory::~ObjectFactory()
{
}
