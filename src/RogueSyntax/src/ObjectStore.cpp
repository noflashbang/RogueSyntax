#include "pch.h"

ObjectStore::ObjectStore()
{
	_store.reserve(500);
}

ObjectStore::~ObjectStore()
{
}

NullObj* ObjectStore::New_NullObj()
{
	auto obj = std::make_shared<NullObj>();
	_store.push_back(obj);
	return obj.get();
}

VoidObj* ObjectStore::New_VoidObj()
{
	auto obj = std::make_shared<VoidObj>();
	_store.push_back(obj);
	return obj.get();
}

BreakObj* ObjectStore::New_BreakObj()
{
	auto obj = std::make_shared<BreakObj>();
	_store.push_back(obj);
	return obj.get();
}

ContinueObj* ObjectStore::New_ContinueObj()
{
	auto obj = std::make_shared<ContinueObj>();
	_store.push_back(obj);
	return obj.get();
}

IntegerObj* ObjectStore::New_IntegerObj(int value)
{
	auto obj = std::make_shared<IntegerObj>(value);
	_store.push_back(obj);
	return obj.get();
}

BooleanObj* ObjectStore::New_BooleanObj(bool value)
{
	auto obj = std::make_shared<BooleanObj>(value);
	_store.push_back(obj);
	return obj.get();
}

StringObj* ObjectStore::New_StringObj(const std::string& value)
{
	auto obj = std::make_shared<StringObj>(value);
	_store.push_back(obj);
	return obj.get();
}

DecimalObj* ObjectStore::New_DecimalObj(float value)
{
	auto obj = std::make_shared<DecimalObj>(value);
	_store.push_back(obj);
	return obj.get();
}

ArrayObj* ObjectStore::New_ArrayObj(const std::vector<const IObject*>& elements)
{
	auto obj = std::make_shared<ArrayObj>(elements);
	_store.push_back(obj);
	return obj.get();
}

HashObj* ObjectStore::New_HashObj(const std::unordered_map<HashKey, HashEntry>& elements)
{
	auto obj = std::make_shared<HashObj>(elements);
	_store.push_back(obj);
	return obj.get();
}

IdentifierObj* ObjectStore::New_IdentifierObj(const std::string& name, const IObject* value)
{
	auto obj = std::make_shared<IdentifierObj>(name, value);
	_store.push_back(obj);
	return obj.get();
}

FunctionObj* ObjectStore::New_FunctionObj(const std::vector<IExpression*>& parameters, const IStatement* body)
{
	auto obj = std::make_shared<FunctionObj>(parameters, body);
	_store.push_back(obj);
	return obj.get();
}

BuiltInObj* ObjectStore::New_BuiltInObj(const std::string& name)
{
	auto obj = std::make_shared<BuiltInObj>(name);
	_store.push_back(obj);
	return obj.get();
}

BuiltInObj* ObjectStore::New_BuiltInObj(const int idx)
{
	auto obj = std::make_shared<BuiltInObj>(idx);
	_store.push_back(obj);
	return obj.get();
}

ErrorObj* ObjectStore::New_ErrorObj(const std::string& message, const ::Token& token)
{
	auto obj = std::make_shared<ErrorObj>(message, token);
	_store.push_back(obj);
	return obj.get();
}

ReturnObj* ObjectStore::New_ReturnObj(const IObject* value)
{
	auto obj = std::make_shared<ReturnObj>(value);
	_store.push_back(obj);
	return obj.get();
}

FunctionCompiledObj* ObjectStore::New_FunctionCompiledObj(const Instructions& instructions, int numLocals, int numParameters)
{
	auto obj = std::make_shared<FunctionCompiledObj>(instructions, numLocals, numParameters);
	_store.push_back(obj);
	return obj.get();
}

ClosureObj* ObjectStore::New_ClosureObj(const FunctionCompiledObj* fun, const std::vector<const IObject*>& free)
{
	auto obj = std::make_shared<ClosureObj>(fun, free);
	_store.push_back(obj);
	return obj.get();
}

void ObjectStore::Add(IObject* obj)
{
	_store.push_back(std::shared_ptr<IObject>(obj));
}

