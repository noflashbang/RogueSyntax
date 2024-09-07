#include "IObject.h"


const ObjectType ObjectType::NULL_OBJ = { 0, "NULL" };
const ObjectType ObjectType::INTEGER_OBJ = { 1, "INTEGER" };
const ObjectType ObjectType::BOOLEAN_OBJ = { 2, "BOOLEAN" };
const ObjectType ObjectType::RETURN_OBJ = { 3, "RETURN" };
const ObjectType ObjectType::ERROR_OBJ = { 4, "ERROR" };
const ObjectType ObjectType::FUNCTION_OBJ = { 5, "FUNCTION" };

std::shared_ptr<BooleanObj> BooleanObj::TRUE_OBJ_REF =  BooleanObj::New(true);
std::shared_ptr<BooleanObj> BooleanObj::FALSE_OBJ_REF = BooleanObj::New(false);


std::shared_ptr<NullObj> NullObj::NULL_OBJ_REF = NullObj::New();

std::shared_ptr<NullObj> NullObj::New()
{
	return std::make_shared<NullObj>();
}

std::shared_ptr<IntegerObj> IntegerObj::New(int value)
{
	return std::make_shared<IntegerObj>(value);
}

std::shared_ptr<BooleanObj> BooleanObj::New(bool value)
{
	return std::make_shared<BooleanObj>(value);
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



