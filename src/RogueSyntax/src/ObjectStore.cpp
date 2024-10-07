#include "pch.h"

//static references
std::shared_ptr<BooleanObj> ObjectStore::TRUE_OBJ = std::make_shared<BooleanObj>(true);
std::shared_ptr<BooleanObj> ObjectStore::FALSE_OBJ = std::make_shared<BooleanObj>(false);;
std::shared_ptr<NullObj> ObjectStore::NULL_OBJ = std::make_shared<NullObj>();
std::shared_ptr<VoidObj> ObjectStore::VOID_OBJ = std::make_shared<VoidObj>();
std::shared_ptr<ContinueObj> ObjectStore::CONTINUE_OBJ = std::make_shared<ContinueObj>();
std::shared_ptr<BreakObj> ObjectStore::BREAK_OBJ = std::make_shared<BreakObj>();

BooleanObj* BooleanObj::TRUE_OBJ_REF = ObjectStore::TRUE_OBJ.get();
BooleanObj* BooleanObj::FALSE_OBJ_REF = ObjectStore::FALSE_OBJ.get();
NullObj* NullObj::NULL_OBJ_REF = ObjectStore::NULL_OBJ.get();
VoidObj* VoidObj::VOID_OBJ_REF = ObjectStore::VOID_OBJ.get();
ContinueObj* ContinueObj::CONTINUE_OBJ_REF = ObjectStore::CONTINUE_OBJ.get();
BreakObj* BreakObj::BREAK_OBJ_REF = ObjectStore::BREAK_OBJ.get();

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
