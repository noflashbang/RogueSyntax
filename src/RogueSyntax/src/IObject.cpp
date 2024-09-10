#include "pch.h"

unsigned int ObjectType::NextObjectType = 0;

const ObjectType ObjectType::NULL_OBJ      = { NextObjectType++, "NULL" };
const ObjectType ObjectType::INTEGER_OBJ   = { NextObjectType++, "INTEGER" };
const ObjectType ObjectType::DECIMAL_OBJ   = { NextObjectType++, "DECIMAL" };
const ObjectType ObjectType::STRING_OBJ    = { NextObjectType++, "STRING" };
const ObjectType ObjectType::BOOLEAN_OBJ   = { NextObjectType++, "BOOLEAN" };
const ObjectType ObjectType::RETURN_OBJ    = { NextObjectType++, "RETURN" };
const ObjectType ObjectType::ERROR_OBJ     = { NextObjectType++, "ERROR" };
const ObjectType ObjectType::FUNCTION_OBJ  = { NextObjectType++, "FUNCTION" };
const ObjectType ObjectType::BUILTIN_OBJ   = { NextObjectType++, "BUILTIN" };
const ObjectType ObjectType::ARRAY_OBJ     = { NextObjectType++, "ARRAY" };

const ObjectType ObjectType::BREAK_OBJ     = { NextObjectType++, "BREAK" };
const ObjectType ObjectType::CONTINUE_OBJ  = { NextObjectType++, "CONTINUE" };

std::shared_ptr<BooleanObj> BooleanObj::TRUE_OBJ_REF =  BooleanObj::New(true);
std::shared_ptr<BooleanObj> BooleanObj::FALSE_OBJ_REF = BooleanObj::New(false);


std::shared_ptr<NullObj> NullObj::NULL_OBJ_REF = NullObj::New();

std::shared_ptr<NullObj> NullObj::New()
{
	return std::make_shared<NullObj>();
}

std::shared_ptr<BreakObj> BreakObj::BREAK_OBJ_REF = BreakObj::New();

std::shared_ptr<BreakObj> BreakObj::New()
{
	return std::make_shared<BreakObj>();
}

std::shared_ptr<ContinueObj> ContinueObj::CONTINUE_OBJ_REF = ContinueObj::New();

std::shared_ptr<ContinueObj> ContinueObj::New()
{
	return std::make_shared<ContinueObj>();
}


std::shared_ptr<IntegerObj> IntegerObj::New(int value)
{
	return std::make_shared<IntegerObj>(value);
}

std::shared_ptr<DecimalObj> DecimalObj::New(float value)
{
	return std::make_shared<DecimalObj>(value);
}

std::shared_ptr<StringObj> StringObj::New(const std::string& value)
{
	return std::make_shared<StringObj>(value);
}

std::shared_ptr<BooleanObj> BooleanObj::New(bool value)
{
	return std::make_shared<BooleanObj>(value);
}

std::shared_ptr<ArrayObj> ArrayObj::New(const std::vector<std::shared_ptr<IObject>>& elements)
{
	return std::make_shared<ArrayObj>(elements);
}

std::shared_ptr<ErrorObj> ErrorObj::New(const std::string& message, const ::Token& token)
{
	return std::make_shared<ErrorObj>(message, token);
}

std::shared_ptr<ReturnObj> ReturnObj::New(const std::shared_ptr<IObject>& value)
{
	return std::make_shared<ReturnObj>(value);
}

std::shared_ptr<FunctionObj> FunctionObj::New(const std::vector<std::shared_ptr<IExpression>>& parameters, const std::shared_ptr<IStatement>& body, const std::shared_ptr<Environment>& env)
{
	return std::make_shared<FunctionObj>(parameters, body, env);
}

std::shared_ptr<BuiltInObj> BuiltInObj::New(const std::string& name)
{
	return std::make_shared<BuiltInObj>(name);
}




