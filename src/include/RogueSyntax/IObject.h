#pragma once

#include "StandardLib.h"
#include "Token.h"
#include "AstNode.h"

struct Environment;

struct ObjectType
{
	const unsigned int Type;
	const std::string Name;

	std::string ToString() const
	{
		return Name;
	}

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

	explicit constexpr operator const unsigned int() const
	{
		return Type;
	}

	explicit constexpr operator const std::string() const
	{
		return Name;
	}

	static unsigned int NextObjectType;
	//TYPES
	static const ObjectType NULL_OBJ;
	static const ObjectType VOID_OBJ;
	static const ObjectType INTEGER_OBJ;
	static const ObjectType DECIMAL_OBJ;
	static const ObjectType STRING_OBJ;
	static const ObjectType BOOLEAN_OBJ;
	static const ObjectType ARRAY_OBJ;
	static const ObjectType HASH_OBJ;
	static const ObjectType IDENT_OBJ;
	static const ObjectType RETURN_OBJ;
	static const ObjectType ERROR_OBJ;
	static const ObjectType FUNCTION_OBJ;
	static const ObjectType BUILTIN_OBJ;

	//special flow control objects
	static const ObjectType BREAK_OBJ;
	static const ObjectType CONTINUE_OBJ;
};

class IObject
{
public:
	virtual const ObjectType& Type() const = 0;
	virtual std::string Inspect() const = 0;
	virtual std::shared_ptr<IObject> Clone() const = 0;

	virtual ~IObject() = default;
};

class IAssignableObject : public IObject
{
public:
	
	virtual std::shared_ptr<IObject> Set(const std::shared_ptr<IObject>& key, const std::shared_ptr<IObject>& value) = 0;
	
	virtual ~IAssignableObject() = default;
};

class NullObj : public IObject
{
public:
	NullObj() { _dummy = 0; }
	virtual ~NullObj() = default;

	const ObjectType& Type() const override
	{
		return ObjectType::NULL_OBJ;
	}

	std::string Inspect() const override
	{
		return "null";
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return NULL_OBJ_REF;
	}

	static std::shared_ptr<NullObj> NULL_OBJ_REF;

protected:
	static std::shared_ptr<NullObj> New();
	
private:
	int _dummy;
};

class VoidObj : public IObject
{
public:
	VoidObj() { _dummy = 0; }
	virtual ~VoidObj() = default;

	const ObjectType& Type() const override
	{
		return ObjectType::NULL_OBJ;
	}

	std::string Inspect() const override
	{
		return "null";
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return VOID_OBJ_REF;
	}

	static std::shared_ptr<VoidObj> VOID_OBJ_REF;

protected:
	static std::shared_ptr<VoidObj> New();

private:
	int _dummy;
};


class BreakObj : public IObject
{
public:
	BreakObj() { _dummy = 0; }
	virtual ~BreakObj() = default;

	const ObjectType& Type() const override
	{
		return ObjectType::BREAK_OBJ;
	}

	std::string Inspect() const override
	{
		return "break";
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return BREAK_OBJ_REF;
	}

	static std::shared_ptr<BreakObj> BREAK_OBJ_REF;
	static std::shared_ptr<BreakObj> New();

private:
	int _dummy;
};

class ContinueObj : public IObject
{
public:
	ContinueObj() { _dummy = 0; }
	virtual ~ContinueObj() = default;
	const ObjectType& Type() const override
	{
		return ObjectType::CONTINUE_OBJ;
	}

	std::string Inspect() const override
	{
		return "continue";
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return CONTINUE_OBJ_REF;
	}

	static std::shared_ptr<ContinueObj> CONTINUE_OBJ_REF;
	static std::shared_ptr<ContinueObj> New();

private:
	int _dummy;
};

class IntegerObj : public IObject
{
public:
	IntegerObj(int value) : Value(value) {};
	virtual ~IntegerObj() = default;

	const ObjectType& Type() const override
	{
		return ObjectType::INTEGER_OBJ;
	}

	std::string Inspect() const override
	{
		return std::to_string(Value);
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return New(Value);
	}

	int32_t Value;

	static std::shared_ptr<IntegerObj> New(int value);
};

class DecimalObj : public IObject
{
public:
	DecimalObj(float value) : Value(value) {};
	virtual ~DecimalObj() = default;

	const ObjectType& Type() const override
	{
		return ObjectType::DECIMAL_OBJ;
	}

	std::string Inspect() const override
	{
		return std::to_string(Value);
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return New(Value);
	}

	float Value;

	static std::shared_ptr<DecimalObj> New(float value);
};

class StringObj : public IObject
{
public:
	StringObj(const std::string& value) : Value(value) {}
	virtual ~StringObj() = default;

	const ObjectType& Type() const override
	{
		return ObjectType::STRING_OBJ;
	}

	std::string Inspect() const override
	{
		return Value;
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return New(Value);
	}

	std::string Value;

	static std::shared_ptr<StringObj> New(const std::string& value);
};

class BooleanObj : public IObject
{
public:
	BooleanObj(bool value) : Value(value) {}
	virtual ~BooleanObj() = default;

	const ObjectType& Type() const override
	{
		return ObjectType::BOOLEAN_OBJ;
	}

	std::string Inspect() const override
	{
		return Value ? "true" : "false";
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return New(Value);
	}

	bool Value;

	//Native object references
	static std::shared_ptr<BooleanObj> TRUE_OBJ_REF;
	static std::shared_ptr<BooleanObj> FALSE_OBJ_REF;


	static std::shared_ptr<BooleanObj> New(bool value);
};

class ArrayObj : public IAssignableObject
{
public:
	ArrayObj(const std::vector<std::shared_ptr<IObject>>& elements) : Elements(elements) {}
	virtual ~ArrayObj() = default;

	const ObjectType& Type() const override
	{
		return ObjectType::ARRAY_OBJ;
	}

	std::string Inspect() const override
	{
		std::string out = "[";

		std::for_each(Elements.begin(), Elements.end(), [&out](const auto& elem)
		{
			out.append(elem->Inspect());
			out.append(", ");
		});

		if (Elements.size() > 0)
		{
			//remove the last comma
			out.pop_back();
			out.pop_back();
		}

		out += "]";
		return out;
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return New(Elements);
	}
	std::shared_ptr<IObject> Set(const std::shared_ptr<IObject>& key, const std::shared_ptr<IObject>& value) override;

	std::vector<std::shared_ptr<IObject>> Elements;

	static std::shared_ptr<ArrayObj> New(const std::vector<std::shared_ptr<IObject>>& elements);
};



struct HashKey
{
	HashKey(ObjectType type, const std::string& key) : Type(std::hash<std::string>{}(type.Name)), Key(std::hash<std::string>{}(key)) {}

	bool operator==(const HashKey& other) const
	{
		return Type == other.Type && Key == other.Key;
	}

	std::size_t Type;
	std::size_t Key;

	std::size_t Hash() const
	{
		return Type ^ Key;
	}
};

namespace std
{
	template <>
	struct hash<HashKey>
	{
		std::size_t operator()(HashKey const& in) const
		{
			return in.Hash();
		}
	};
}

struct HashEntry
{
	std::shared_ptr<IObject> Key;
	std::shared_ptr<IObject> Value;
};

class HashObj : public IAssignableObject
{
public:
	HashObj(const std::unordered_map<HashKey, HashEntry>& elements) : Elements(elements) {}
	virtual ~HashObj() = default;

	const ObjectType& Type() const override
	{
		return ObjectType::HASH_OBJ;
	}

	std::string Inspect() const override
	{
		std::string out = "{";

		std::for_each(Elements.begin(), Elements.end(), [&out](const auto& elem)
			{
				auto [key, value] = elem;
				out.append(value.Key->Inspect());
				out.append(": ");
				out.append(value.Value->Inspect());
				out.append(", ");
			});

		if (Elements.size() > 0)
		{
			//remove the last comma
			out.pop_back();
			out.pop_back();
		}

		out += "}";
		return out;
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return New(Elements);
	}

	std::shared_ptr<IObject> Set(const std::shared_ptr<IObject>& key, const std::shared_ptr<IObject>& value) override;

	std::unordered_map<HashKey, HashEntry> Elements;

	static std::shared_ptr<HashObj> New(const std::unordered_map<HashKey, HashEntry>& elements);
};

class IdentifierObj : public IAssignableObject
{
public:
	IdentifierObj(const std::string& name, const std::shared_ptr<IObject>& value) : Name(name), Value(value) {}
	virtual ~IdentifierObj() = default;

	const ObjectType& Type() const override
	{
		return ObjectType::IDENT_OBJ;
	}

	std::string Inspect() const override
	{
		return Value->Inspect();
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return New(Name, Value->Clone());
	}

	std::shared_ptr<IObject> Set(const std::shared_ptr<IObject>& key, const std::shared_ptr<IObject>& value) override;

	std::string Name;
	std::shared_ptr<IObject> Value;

	static std::shared_ptr<IdentifierObj> New(const std::string& name, const std::shared_ptr<IObject>& value);

private:

};

class ReturnObj : public IObject
{
public:
	ReturnObj(const std::shared_ptr<IObject>& value) : Value(value) {}
	virtual ~ReturnObj() = default;

	const ObjectType& Type() const override
	{
		return ObjectType::RETURN_OBJ;
	}

	std::string Inspect() const override
	{
		return Value->Inspect();
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return New(Value->Clone());
	}

	std::shared_ptr<IObject> Value;

	static std::shared_ptr<ReturnObj> New(const std::shared_ptr<IObject>& value);
};

class ErrorObj : public IObject
{
public:
	ErrorObj(const std::string& message, const Token& token) : Message(message), Token(token) {}
	virtual ~ErrorObj() = default;

	const ObjectType& Type() const override
	{
		return ObjectType::ERROR_OBJ;
	}

	std::string Inspect() const override
	{
		return "ERROR: " + Message;
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return New(Message, Token);
	}

	std::string Message;
	Token Token;

	static std::shared_ptr<ErrorObj> New(const std::string& message, const ::Token& token);
};

class FunctionObj : public IObject
{
public:
	FunctionObj(const std::vector<std::shared_ptr<IExpression>>& parameters, const std::shared_ptr<IStatement>& body) : Parameters(parameters), Body(body) {}
	virtual ~FunctionObj() = default;

	const ObjectType& Type() const override
	{
		return ObjectType::FUNCTION_OBJ;
	}

	std::string Inspect() const override
	{
		std::string out = "fn(";

		std::for_each(Parameters.begin(), Parameters.end(), [&out](const auto& param)
		{
			out.append(param->ToString());
			out.append(", ");
		});

		if (Parameters.size() > 0)
		{
			//remove the last comma
			out.pop_back();
			out.pop_back();
		}

		out += ") {\n";
		out += Body->ToString();
		out += "\n}";
		return out;
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return New(Parameters, Body);
	}

	std::vector<std::shared_ptr<IExpression>> Parameters;
	std::shared_ptr<IStatement> Body;

	static std::shared_ptr<FunctionObj> New(const std::vector<std::shared_ptr<IExpression>>& parameters, const std::shared_ptr<IStatement>& body);
};

class BuiltInObj : public IObject
{
public:
	BuiltInObj(const std::string& name) : Name(name) {}
	virtual ~BuiltInObj() = default;

	const ObjectType& Type() const override
	{
		return ObjectType::BUILTIN_OBJ;
	}

	std::string Inspect() const override
	{
		return "builtin function";
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return New(Name);
	}

	std::string Name;
	static std::shared_ptr<BuiltInObj> New(const std::string& name);
};
