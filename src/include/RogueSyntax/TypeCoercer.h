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
	std::tuple<std::shared_ptr<IObject>, std::shared_ptr<IObject>> CoerceTypes(const IObject* const left, const IObject* const right) const;
	std::shared_ptr<IObject>  CoerceThis(const IObject* const source, const IObject* const target) const;

	std::shared_ptr<IObject> EvalAsBoolean(const IObject* const obj) const;
	std::shared_ptr<IObject> EvalAsDecimal(const IObject* const obj) const;
	std::shared_ptr<IObject> EvalAsInteger(const IObject* const obj) const;
	std::shared_ptr<IObject> EvalAsString(const IObject* const obj) const;

private:
	std::unordered_map<std::size_t, std::function<std::shared_ptr<IObject>(const IObject* const right)>> _coercionMap;
	std::unordered_map<std::size_t, std::map<std::size_t, std::size_t>> _coercionTable;
};

