#pragma once

#include "StandardLib.h"

struct Environment;

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
	static const ObjectType FUNCTION_OBJ;

	//special flow control objects
	static const ObjectType BREAK_OBJ;
	static const ObjectType CONTINUE_OBJ;
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

	static std::shared_ptr<NullObj> NULL_OBJ_REF;
	static std::shared_ptr<NullObj> New();
	
private:
	int _dummy;
};

class BreakObj : public IObject
{
public:
	BreakObj() { _dummy = 0; }

	ObjectType Type() const override
	{
		return ObjectType::BREAK_OBJ;
	}

	std::string Inspect() const override
	{
		return "break";
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

	ObjectType Type() const override
	{
		return ObjectType::CONTINUE_OBJ;
	}

	std::string Inspect() const override
	{
		return "break";
	}

	static std::shared_ptr<ContinueObj> CONTINUE_OBJ_REF;
	static std::shared_ptr<ContinueObj> New();

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

	static std::shared_ptr<IntegerObj> New(int value);
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
	static std::shared_ptr<BooleanObj> TRUE_OBJ_REF;
	static std::shared_ptr<BooleanObj> FALSE_OBJ_REF;


	static std::shared_ptr<BooleanObj> New(bool value);
};

class ReturnObj : public IObject
{
public:
	ReturnObj(const std::shared_ptr<IObject>& value) : Value(value) {}

	ObjectType Type() const override
	{
		return ObjectType::RETURN_OBJ;
	}

	std::string Inspect() const override
	{
		return Value->Inspect();
	}

	std::shared_ptr<IObject> Value;

	static std::shared_ptr<ReturnObj> New(const std::shared_ptr<IObject>& value);
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

	static std::shared_ptr<ErrorObj> New(const std::string& message, const ::Token& token);
};

class FunctionObj : public IObject
{
public:
	FunctionObj(const std::vector<std::shared_ptr<IExpression>>& parameters, const std::shared_ptr<IStatement>& body, const std::shared_ptr<Environment>& env) : Parameters(parameters), Body(body), Env(env) {}

	ObjectType Type() const override
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

		//remove the last comma
		out.pop_back();
		out.pop_back();

		out += ") {\n";
		out += Body->ToString();
		out += "\n}";
		return out;
	}

	std::vector<std::shared_ptr<IExpression>> Parameters;
	std::shared_ptr<IStatement> Body;
	std::shared_ptr<Environment> Env;

	static std::shared_ptr<FunctionObj> New(const std::vector<std::shared_ptr<IExpression>>& parameters, const std::shared_ptr<IStatement>& body, const std::shared_ptr<Environment>& env);
};

