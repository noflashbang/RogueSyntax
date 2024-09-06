#pragma once

#include "StandardLib.h"


struct ObjectType
{
	unsigned int Type;
	std::string Name;

	constexpr bool operator==(const ObjectType& other) const
	{
		return Type == other.Type;
	}

	constexpr bool operator!=(const ObjectType& other) const
	{
		return Type != other.Type;
	}

	constexpr bool operator<(const ObjectType& other) const
	{
		return Type < other.Type;
	}

	constexpr bool operator>(const ObjectType& other) const
	{
		return Type > other.Type;
	}

	constexpr bool operator<=(const ObjectType& other) const
	{
		return Type <= other.Type;
	}

	constexpr bool operator>=(const ObjectType& other) const
	{
		return Type >= other.Type;
	}

	explicit constexpr operator unsigned int() const
	{
		return Type;
	}

	explicit constexpr operator std::string() const
	{
		return Name;
	}

	//TYPES
	static const ObjectType NULL_OBJ;
	static const ObjectType INTEGER_OBJ;
	static const ObjectType BOOLEAN_OBJ;
	static const ObjectType RETURN_OBJ;
	static const ObjectType ERROR_OBJ;
};

class IObject
{
public:
	virtual ObjectType Type() const = 0;
	virtual std::string Inspect() const = 0;

	virtual ~IObject() = default;
};

class NullObj : public IObject
{
public:
	NullObj() { _dummy = 0; }

	ObjectType Type() const override
	{
		return ObjectType::NULL_OBJ;
	}

	std::string Inspect() const override
	{
		return "null";
	}
	static NullObj NULL_OBJ_REF;

private:
	int _dummy;
};

class IntegerObj : public IObject
{
public:
	IntegerObj(int value) : Value(value) {}

	ObjectType Type() const override
	{
		return ObjectType::INTEGER_OBJ;
	}

	std::string Inspect() const override
	{
		return std::to_string(Value);
	}

	int32_t Value;
};

class BooleanObj : public IObject
{
public:
	BooleanObj(bool value) : Value(value) {}

	ObjectType Type() const override
	{
		return ObjectType::BOOLEAN_OBJ;
	}

	std::string Inspect() const override
	{
		return Value ? "true" : "false";
	}

	bool Value;

	//Native object references
	static BooleanObj TRUE_OBJ_REF;
	static BooleanObj FALSE_OBJ_REF;
};

class ReturnObj : public IObject
{
public:
	ReturnObj(IObject* value) : Value(value) {}

	ObjectType Type() const override
	{
		return ObjectType::RETURN_OBJ;
	}

	std::string Inspect() const override
	{
		return Value->Inspect();
	}

	IObject* Value;
};

class ErrorObj : public IObject
{
public:
	ErrorObj(const std::string& message, const Token& token) : Message(message), Token(token) {}

	ObjectType Type() const override
	{
		return ObjectType::ERROR_OBJ;
	}

	std::string Inspect() const override
	{
		return "ERROR: " + Message;
	}

	std::string Message;
	Token Token;
};
