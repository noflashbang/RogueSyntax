#pragma once

#include "StandardLib.h"
#include <IObject.h>
#include <Token.h>

class TypeCoercer
{
public:
	TypeCoercer(const std::shared_ptr<ObjectFactory>& factory);
	~TypeCoercer();

	//convertion functions - the left hand type is the type of the object that the coercion is being applied to
	bool CanCoerceTypes(const IObject* const left, const IObject* const right) const;
	std::tuple<IObject*, IObject*> CoerceTypes(const IObject* const left, const IObject* const right) const;
	IObject*  CoerceThis(const IObject* const source, const IObject* const target) const;

	IObject* EvalAsBoolean(const IObject* const obj) const;
	IObject* EvalAsDecimal(const IObject* const obj) const;
	IObject* EvalAsInteger(const IObject* const obj) const;
	IObject* EvalAsString(const IObject* const obj) const;

private:
	std::unordered_map<std::size_t, std::function<IObject*(const IObject* const obj)>> _coercionMap;
	std::unordered_map<std::size_t, std::map<std::size_t, std::size_t>> _coercionTable;
	std::shared_ptr<ObjectFactory> _factory;
};

