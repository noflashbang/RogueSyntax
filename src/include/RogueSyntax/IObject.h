#pragma once

#include "StandardLib.h"
#include "Token.h"
#include "AstNode.h"
#include "OpCode.h"

class BuiltIn;
class Environment;
class ObjectFactory;

class IObject: public IUnquielyIdentifiable
{
public:
	inline const std::size_t Type() const noexcept { return IUnquielyIdentifiable::Tag(); }
	virtual std::string Inspect() const = 0;

	virtual IObject* Clone(const ObjectFactory* factory) const = 0;

	virtual ~IObject() = default;
};

class IAssignableObject : public IObject
{
public:
	virtual const IObject* Set(const IObject* key, const IObject* value) = 0;
	virtual ~IAssignableObject() = default;
};

class NullObj : public IObject
{
public:
	NullObj() { _dummy = 0; SetUniqueId(this); }
	virtual ~NullObj() = default;

	std::string Inspect() const override
	{
		return "null";
	}
	virtual IObject* Clone(const ObjectFactory* factory) const override;

	static NullObj* NULL_OBJ_REF;

private:
	int _dummy;
};

class VoidObj : public IObject
{
public:
	VoidObj() { _dummy = 0; SetUniqueId(this); }
	virtual ~VoidObj() = default;

	std::string Inspect() const override
	{
		return "null";
	}
	virtual IObject* Clone(const ObjectFactory* factory) const override;
	static VoidObj* VOID_OBJ_REF;

protected:


private:
	int _dummy;
};


class BreakObj : public IObject
{
public:
	BreakObj() { _dummy = 0; SetUniqueId(this); }
	virtual ~BreakObj() = default;

	std::string Inspect() const override
	{
		return "break";
	}
	virtual IObject* Clone(const ObjectFactory* factory) const override;
	static BreakObj* BREAK_OBJ_REF;


private:
	int _dummy;
};

class ContinueObj : public IObject
{
public:
	ContinueObj() { _dummy = 0; SetUniqueId(this); }
	virtual ~ContinueObj() = default;

	std::string Inspect() const override
	{
		return "continue";
	}
	virtual IObject* Clone(const ObjectFactory* factory) const override;
	static ContinueObj* CONTINUE_OBJ_REF;

private:
	int _dummy;
};

class IntegerObj : public IObject
{
public:
	IntegerObj(int value) : Value(value) { SetUniqueId(this); };
	virtual ~IntegerObj() = default;

	std::string Inspect() const override
	{
		return std::to_string(Value);
	}

	virtual IObject* Clone(const ObjectFactory* factory) const override;
	int32_t Value;
};

class DecimalObj : public IObject
{
public:
	DecimalObj(float value) : Value(value) { SetUniqueId(this); };
	virtual ~DecimalObj() = default;

	std::string Inspect() const override
	{
		return std::to_string(Value);
	}
	virtual IObject* Clone(const ObjectFactory* factory) const override;
	float Value;
};

class StringObj : public IObject
{
public:
	StringObj(const std::string& value) : Value(value) { SetUniqueId(this); }
	virtual ~StringObj() = default;

	std::string Inspect() const override
	{
		return Value;
	}
	virtual IObject* Clone(const ObjectFactory* factory) const override;
	std::string Value;
};

class BooleanObj : public IObject
{
public:
	BooleanObj(bool value) : Value(value) { SetUniqueId(this); }
	virtual ~BooleanObj() = default;

	std::string Inspect() const override
	{
		return Value ? "true" : "false";
	}
	virtual IObject* Clone(const ObjectFactory* factory) const override;
	bool Value;

	//Native object references
	static BooleanObj* TRUE_OBJ_REF;
	static BooleanObj* FALSE_OBJ_REF;
};

class ArrayObj : public IAssignableObject
{
public:
	ArrayObj(const std::vector<const IObject*>& elements) : Elements(elements) { SetUniqueId(this); }
	virtual ~ArrayObj() = default;

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
	virtual IObject* Clone(const ObjectFactory* factory) const override;
	const IObject* Set(const IObject* key, const IObject* value) override;
	std::vector<const IObject*> Elements;
};

struct HashKey
{
	HashKey(std::size_t type, const std::string& key) : Type(type), Key(std::hash<std::string>{}(key)) {}

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
	const IObject* Key;
	const IObject* Value;
};

class HashObj : public IAssignableObject
{
public:
	HashObj(const std::unordered_map<HashKey, HashEntry>& elements) : Elements(elements) { SetUniqueId(this); }
	virtual ~HashObj() = default;

	std::string Inspect() const override
	{
		std::string out = "{";

		std::for_each(Elements.begin(), Elements.end(), [&out](const auto& elem)
			{
				auto& [key, value] = elem;
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
	virtual IObject* Clone(const ObjectFactory* factory) const override;
	const IObject* Set(const IObject* key, const IObject* value) override;
	std::unordered_map<HashKey, HashEntry> Elements;
};

class IdentifierObj : public IAssignableObject
{
public:
	IdentifierObj(const std::string& name, const IObject* value) : Name(name), Value(value) { SetUniqueId(this); }
	virtual ~IdentifierObj() = default;

	std::string Inspect() const override
	{
		return Value->Inspect();
	}

	virtual IObject* Clone(const ObjectFactory* factory) const override;

	const IObject* Set(const IObject* key, const IObject* value) override;

	std::string Name;
	const IObject* Value;
};

class ReturnObj : public IObject
{
public:
	ReturnObj(const IObject* value) : Value(value) { SetUniqueId(this); }
	virtual ~ReturnObj() = default;

	std::string Inspect() const override
	{
		return Value->Inspect();
	}

	virtual IObject* Clone(const ObjectFactory* factory) const override;
	const IObject* Value;
};

class ErrorObj : public IObject
{
public:
	ErrorObj(const std::string& message, const Token& token) : Message(message), Token(token) { SetUniqueId(this); }
	virtual ~ErrorObj() = default;

	std::string Inspect() const override
	{
		return "ERROR: " + Message;
	}

	virtual IObject* Clone(const ObjectFactory* factory) const override;

	std::string Message;
	Token Token;
};

class FunctionObj : public IObject
{
public:
	FunctionObj(const std::vector<IExpression*>& parameters, const IStatement* body) : Parameters(parameters), Body(body) { SetUniqueId(this); }
	virtual ~FunctionObj() = default;

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

	virtual IObject* Clone(const ObjectFactory* factory) const override;

	std::vector<IExpression*> Parameters;
	const IStatement* Body;
};

class BuiltInObj : public IObject
{
public:
	BuiltInObj(const std::string& name) : Name(name), Idx(-1) { SetUniqueId(this); }
	BuiltInObj(const int idx) : Name(""), Idx(idx) { SetUniqueId(this); }
	virtual ~BuiltInObj() = default;

	std::function<IObject*(const std::vector<const IObject*>& args)> Resolve(std::shared_ptr<BuiltIn> externals) const;

	std::string Inspect() const override
	{
		return "builtin function";
	}

	virtual IObject* Clone(const ObjectFactory* factory) const override;

	std::string Name;
	int Idx;
};

class FunctionCompiledObj : public IObject
{
public:
	FunctionCompiledObj(const Instructions& instructions, int numLocals, int numParameters) : FuncInstructions(instructions), NumLocals(numLocals), NumParameters(numParameters) { SetUniqueId(this); }
	virtual ~FunctionCompiledObj() = default;

	std::string Inspect() const override
	{
		return OpCode::PrintInstructions(FuncInstructions);
	}

	virtual IObject* Clone(const ObjectFactory* factory) const override;
	
	Instructions FuncInstructions;
	int NumLocals;
	int NumParameters;
};

class ClosureObj : public IObject
{
public:
	ClosureObj(const FunctionCompiledObj* fun, const std::vector<const IObject*>& free) : Function(fun), Frees(free) { SetUniqueId(this); }
	virtual ~ClosureObj() = default;

	std::string Inspect() const override
	{
		return Function->Inspect();
	}

	virtual IObject* Clone(const ObjectFactory* factory) const override;

	const FunctionCompiledObj* Function;
	std::vector<const IObject*> Frees;
};
