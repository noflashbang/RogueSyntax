#pragma once

#include "StandardLib.h"
#include <IObject.h>
#include <Token.h>

class TypeCoercer
{
public:
	TypeCoercer();
	~TypeCoercer();

	//convertion functions - the left hand type is the type of the object that the coercion is being applied to
	bool CanCoerceTypes(const IObject* const left, const IObject* const right) const;
	std::tuple<IObject*, IObject*> CoerceTypes(ObjectStore& store, const IObject* const left, const IObject* const right) const;
	IObject*  CoerceThis(ObjectStore& store, const IObject* const source, const IObject* const target) const;

	IObject* EvalAsBoolean(ObjectStore& store, const IObject* const obj) const;
	IObject* EvalAsDecimal(ObjectStore& store, const IObject* const obj) const;
	IObject* EvalAsInteger(ObjectStore& store, const IObject* const obj) const;
	IObject* EvalAsString(ObjectStore& store, const IObject* const obj) const;

private:
	std::unordered_map<std::size_t, std::function<IObject*(ObjectStore& store, const IObject* const obj)>> _coercionMap;
	std::unordered_map<std::size_t, std::map<std::size_t, std::size_t>> _coercionTable;
};

