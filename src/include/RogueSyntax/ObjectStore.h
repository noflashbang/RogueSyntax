#pragma once

#include "StandardLib.h"
#include "Identifiable.h"
#include "IObject.h"

class ObjectStore
{
public:
	ObjectStore();
	~ObjectStore();

	NullObj* New_NullObj();
	VoidObj* New_VoidObj();
	BreakObj* New_BreakObj();
	ContinueObj* New_ContinueObj();
	IntegerObj* New_IntegerObj(int value);
	BooleanObj* New_BooleanObj(bool value);
	StringObj* New_StringObj(const std::string& value);
	DecimalObj* New_DecimalObj(float value);
	ArrayObj* New_ArrayObj(const std::vector<const IObject*>& elements);
	HashObj* New_HashObj(const std::unordered_map<HashKey, HashEntry>& elements);
	IdentifierObj* New_IdentifierObj(const std::string& name, const IObject* value);
	FunctionObj* New_FunctionObj(const std::vector<IExpression*>& parameters, const IStatement* body);
	BuiltInObj* New_BuiltInObj(const std::string& name);
	BuiltInObj* New_BuiltInObj(const int idx);
	ErrorObj* New_ErrorObj(const std::string& message, const ::Token& token);
	ReturnObj* New_ReturnObj(const IObject* value);
	FunctionCompiledObj* New_FunctionCompiledObj(const Instructions& instructions, int numLocals, int numParameters);
	ClosureObj* New_ClosureObj(const FunctionCompiledObj* fun, const std::vector<const IObject*>& free);

	void Add(IObject* obj);

private:
	std::vector<std::shared_ptr<IObject>> _store;
};

