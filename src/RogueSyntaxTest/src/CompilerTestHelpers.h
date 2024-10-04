#include <vector>
#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

typedef std::variant<int, std::string, float, bool, NullObj, std::shared_ptr<ArrayObj>, std::shared_ptr<HashObj>, std::shared_ptr<FunctionCompiledObj>> ConstantValue;


bool TestIObjects(const IObject* expected, const IObject* actual);

template<typename T, typename R>
bool TestConstantValues(ConstantValue expected, const IObject* actual)
{
	if (std::holds_alternative<T>(expected))
	{
		if (actual->Type() == typeid(R).hash_code())
		{
			if (std::get<T>(expected) != dynamic_cast<const R*>(actual)->Value)
			{
				throw std::runtime_error(std::format("Expected and actual constant values are not the same. Expected={} Actual={}", std::get<T>(expected), dynamic_cast<const R*>(actual)->Value));
			}
		}
		else
		{
			throw std::runtime_error(std::format("Got wrong constant type. Expected={} Got={}", typeid(R).name(), actual->TypeName()));
		}
	}
	return true;
}

bool TestConstant(const ConstantValue& expected, const IObject* actual);

bool TestInstructions(const Instructions& expected, const Instructions& actual);

bool TestConstants(const std::vector<ConstantValue>& expected, const std::vector<const IObject*>& actual);

Instructions ConcatInstructions(const std::vector<Instructions>& instructions);

bool TestByteCode(const std::vector<ConstantValue>& expectedConstants, const std::vector<Instructions>& expectedInstructions, const ByteCode& actual);

bool CompilerTest(const std::vector<ConstantValue>& expectedConstants, const std::vector<Instructions>& expectedInstructions, std::string input);

bool VmTest(std::string input, ConstantValue expected);