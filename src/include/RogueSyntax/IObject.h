#pragma once

#include "StandardLib.h"
#include "Token.h"
#include "AstNode.h"
#include "OpCode.h"

class BuiltIn;
struct Environment;

class IObject: public IUnquielyIdentifiable
{
public:
	inline const std::size_t Type() const noexcept { return IUnquielyIdentifiable::Tag(); }
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
	NullObj() { _dummy = 0; SetUniqueId(this); }
	virtual ~NullObj() = default;

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
	VoidObj() { _dummy = 0; SetUniqueId(this); }
	virtual ~VoidObj() = default;

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
	BreakObj() { _dummy = 0; SetUniqueId(this); }
	virtual ~BreakObj() = default;

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
	ContinueObj() { _dummy = 0; SetUniqueId(this); }
	virtual ~ContinueObj() = default;

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
	IntegerObj(int value) : Value(value) { SetUniqueId(this); };
	virtual ~IntegerObj() = default;

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
	DecimalObj(float value) : Value(value) { SetUniqueId(this); };
	virtual ~DecimalObj() = default;

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
	StringObj(const std::string& value) : Value(value) { SetUniqueId(this); }
	virtual ~StringObj() = default;

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
	BooleanObj(bool value) : Value(value) { SetUniqueId(this); }
	virtual ~BooleanObj() = default;

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
	ArrayObj(const std::vector<std::shared_ptr<IObject>>& elements) : Elements(elements) { SetUniqueId(this); }
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
	std::shared_ptr<IObject> Key;
	std::shared_ptr<IObject> Value;
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
	IdentifierObj(const std::string& name, const std::shared_ptr<IObject>& value) : Name(name), Value(value) { SetUniqueId(this); }
	virtual ~IdentifierObj() = default;

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
	ReturnObj(const std::shared_ptr<IObject>& value) : Value(value) { SetUniqueId(this); }
	virtual ~ReturnObj() = default;

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
	ErrorObj(const std::string& message, const Token& token) : Message(message), Token(token) { SetUniqueId(this); }
	virtual ~ErrorObj() = default;

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

	std::shared_ptr<IObject> Clone() const override
	{
		return New(Parameters, Body);
	}

	std::vector<IExpression*> Parameters;
	const IStatement* Body;

	static std::shared_ptr<FunctionObj> New(const std::vector<IExpression*>& parameters, const IStatement* body);
};

class BuiltInObj : public IObject
{
public:
	BuiltInObj(const std::string& name) : Name(name), Idx(-1) { SetUniqueId(this); }
	BuiltInObj(const int idx) : Name(""), Idx(idx) { SetUniqueId(this); }
	virtual ~BuiltInObj() = default;

	std::function<std::shared_ptr<IObject>(const std::vector<std::shared_ptr<IObject>>& args)> Resolve(std::shared_ptr<BuiltIn> externals) const;

	std::string Inspect() const override
	{
		return "builtin function";
	}

	std::shared_ptr<IObject> Clone() const override
	{
		if (Idx != -1)
		{
			return New(Idx);
		}
		return New(Name);
	}

	std::string Name;
	int Idx;

	static std::shared_ptr<BuiltInObj> New(const std::string& name);
	static std::shared_ptr<BuiltInObj> New(const int idx);
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

	std::shared_ptr<IObject> Clone() const override
	{
		return New(FuncInstructions, NumLocals, NumParameters);
	}
	
	Instructions FuncInstructions;
	int NumLocals;
	int NumParameters;

	static std::shared_ptr<FunctionCompiledObj> New(const Instructions& instructions, int numLocals, int numParameters);
};

class ClosureObj : public IObject
{
public:
	ClosureObj(const std::shared_ptr<FunctionCompiledObj>& fun, const std::vector<std::shared_ptr<IObject>>& free) : Function(fun), Frees(free) { SetUniqueId(this); }
	virtual ~ClosureObj() = default;

	std::string Inspect() const override
	{
		return Function->Inspect();
	}

	std::shared_ptr<IObject> Clone() const override
	{
		return New(Function, Frees);
	}

	std::shared_ptr<FunctionCompiledObj> Function;
	std::vector<std::shared_ptr<IObject>> Frees;
	
	static std::shared_ptr<ClosureObj> New(const std::shared_ptr<FunctionCompiledObj>& fun, const std::vector<std::shared_ptr<IObject>>& free);
};
