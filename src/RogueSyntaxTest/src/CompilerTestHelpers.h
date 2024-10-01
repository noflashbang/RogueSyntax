#include <vector>
#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

typedef std::variant<int, std::string, float, bool, NullObj, std::shared_ptr<ArrayObj>, std::shared_ptr<HashObj>, std::shared_ptr<FunctionCompiledObj>> ConstantValue;


bool TestIObjects(std::shared_ptr<IObject> expected, std::shared_ptr<IObject> actual);

template<typename T, typename R>
bool TestConstantValues(ConstantValue expected, std::shared_ptr<IObject> actual)
{
	if (std::holds_alternative<T>(expected))
	{
		if (typeid(*(actual.get())) == typeid(R))
		{
			if (std::get<T>(expected) != std::dynamic_pointer_cast<R>(actual)->Value)
			{
				throw std::runtime_error(std::format("Expected and actual constant values are not the same. Expected={} Actual={}", std::get<T>(expected), std::dynamic_pointer_cast<R>(actual)->Value));
			}
		}
		else
		{
			throw std::runtime_error(std::format("Got wrong constant type. Expected={} Got={}", typeid(R).name(), typeid(*(actual.get())).name()));
		}
	}
	return true;
}

bool TestConstant(const ConstantValue& expected, const std::shared_ptr<IObject>& actual);

bool TestInstructions(const Instructions& expected, const Instructions& actual);

bool TestConstants(const std::vector<ConstantValue>& expected, const std::vector<std::shared_ptr<IObject>>& actual);

Instructions ConcatInstructions(const std::vector<Instructions>& instructions);

bool TestByteCode(const std::vector<ConstantValue>& expectedConstants, const std::vector<Instructions>& expectedInstructions, const ByteCode& actual);

bool CompilerTest(const std::vector<ConstantValue>& expectedConstants, const std::vector<Instructions>& expectedInstructions, std::string input);

bool VmTest(std::string input, ConstantValue expected);