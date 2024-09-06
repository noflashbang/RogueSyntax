#include "IObject.h"


const ObjectType ObjectType::NULL_OBJ = { 0, "NULL" };
const ObjectType ObjectType::INTEGER_OBJ = { 1, "INTEGER" };
const ObjectType ObjectType::BOOLEAN_OBJ = { 2, "BOOLEAN" };
const ObjectType ObjectType::RETURN_OBJ = { 3, "RETURN" };
const ObjectType ObjectType::ERROR_OBJ = { 4, "ERROR" };

BooleanObj BooleanObj::TRUE_OBJ_REF = {true};
BooleanObj BooleanObj::FALSE_OBJ_REF = {false};
NullObj NullObj::NULL_OBJ_REF = {};

