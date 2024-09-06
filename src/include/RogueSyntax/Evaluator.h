#pragma once

#include "StandardLib.h"
#include "IObject.h"
#include "Token.h"
#include "AstNode.h"

class Evaluator
{
public:
	IObject* Eval(Program& program);
	IObject* Eval(INode* node);
};

