#pragma once

#include <RogueSyntaxCore.h>

//class that acts as the API
class RogueSyntax
{
public:
	RogueSyntax();
	~RogueSyntax();

	std::shared_ptr<Program> Parse(const std::string& input, const std::string& unit) const;
	ByteCode Compile(const std::string& input, const std::string& unit) const;
	std::shared_ptr<RogueVM> MakeVM(ByteCode code) const;
	const IObject* QuickEval(EvaluatorType type, const std::string& input) const;
	void RegisterBuiltIn(const std::string& name, std::function<IObject* (const ObjectFactory* factory, const std::vector<const IObject*>& args)> func);

private:
	std::shared_ptr<Evaluator> MakeEvaluator(EvaluatorType type) const;
	std::shared_ptr<ObjectStore> _objectStore;
	std::shared_ptr<BuiltIn> _builtIn;
};

