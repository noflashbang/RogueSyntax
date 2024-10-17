#include <vector>
#include "CompilerTestHelpers.h"
#include <RogueSyntaxCore.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <RogueSyntax.h>

bool TestIObjects(const IObject* expected, const IObject* actual)
{
	if (expected->Type() != actual->Type())
	{
		throw std::runtime_error(std::format("Expected and actual object types are not the same. Expected={} Actual={}", expected->TypeName(), actual->TypeName()));
	}
	if (expected->Inspect() != actual->Inspect())
	{
		throw std::runtime_error(std::format("Expected and actual object values are not the same. Expected={} Actual={}", expected->Inspect(), actual->Inspect()));
	}
	return true;
}

bool TestConstant(const ConstantValue& expected, const IObject* actual)
{
	if (std::holds_alternative<int>(expected))
	{
		return TestConstantValues<int, IntegerObj>(expected, actual);
	}
	else if (std::holds_alternative<std::string>(expected))
	{
		return TestConstantValues<std::string, StringObj>(expected, actual);
	}
	else if (std::holds_alternative<float>(expected))
	{
		return TestConstantValues<float, DecimalObj>(expected, actual);
	}
	else if (std::holds_alternative<bool>(expected))
	{
		return TestConstantValues<bool, BooleanObj>(expected, actual);
	}
	else if (std::holds_alternative<NullObj>(expected))
	{
		if (actual->Type() != typeid(NullObj).hash_code())
		{
			throw std::runtime_error(std::format("Expected and actual constant values are not the same. Expected={} Actual={}", "null", actual->Inspect()));
		}
	}
	else if (std::holds_alternative<std::shared_ptr<ArrayObj>>(expected))
	{
		auto arr = std::get<std::shared_ptr<ArrayObj>>(expected);
		auto actualArr = dynamic_cast<const ArrayObj*>(actual);

		for (int i = 0; i < arr->Elements.size(); i++)
		{
			auto expectedValue = arr->Elements[i];
			auto actualValue = actualArr->Elements[i];
			TestIObjects(expectedValue, actualValue);
		}
	}
	else if (std::holds_alternative<std::shared_ptr<HashObj>>(expected))
	{
		auto hash = std::get<std::shared_ptr<HashObj>>(expected);
		auto actualHash = dynamic_cast<const HashObj*>(actual);

		for (const auto& [key, value] : hash->Elements)
		{
			const IObject* actualValue = NullObj::NULL_OBJ_REF;

			auto entry = actualHash->Elements.find(key);
			if (entry != actualHash->Elements.end())
			{
				actualValue = entry->second.Value;
			}

			TestIObjects(value.Value, actualValue);
		}
	}
	else if (std::holds_alternative<std::shared_ptr<FunctionCompiledObj>>(expected))
	{
		auto expectedValue = std::get<std::shared_ptr<FunctionCompiledObj>>(expected);
		auto actualValue = dynamic_cast<const FunctionCompiledObj*>(actual);

		TestInstructions(expectedValue->FuncInstructions, actualValue->FuncInstructions);
	}
	else
	{
		throw std::runtime_error(std::format("Unknown constant type."));
	}
	return true;
}

bool TestInstructions(const Instructions& expected, const Instructions& actual)
{
	if (expected.size() != actual.size())
	{
		const auto print = OpCode::PrintInstuctionsCompared(expected, actual);
		throw std::runtime_error(std::format("Expected and actual number of instructions are not the same size. Expected={} Actual={}\n{}", expected.size(), actual.size(), print));
	}

	for (size_t i = 0; i < expected.size(); i++)
	{
		if (expected[i] != actual[i])
		{
			const auto print = OpCode::PrintInstuctionsCompared(expected, actual);
			throw std::runtime_error(std::format("Expected and actual instructions are not the same @offset{}. Expected={} Actual={}\n{}", i, expected[i], actual[i], print));
		}
	}
	return true;
}

bool TestConstants(const std::vector<ConstantValue>& expected, const std::vector<const IObject*>& actual)
{
	if (expected.size() != actual.size())
	{
		throw std::runtime_error(std::format("Expected and actual number of constants are not the same size. Expected={} Actual={}", expected.size(), actual.size()));
	}

	for (size_t i = 0; i < expected.size(); i++)
	{
		TestConstant(expected[i], actual[i]);
	}
	return true;
}

Instructions ConcatInstructions(const std::vector<Instructions>& instructions)
{
	Instructions result;
	for (const auto& instruction : instructions)
	{
		result.insert(result.end(), instruction.begin(), instruction.end());
	}
	return result;
}

bool TestByteCode(const std::vector<ConstantValue>& expectedConstants, const std::vector<Instructions>& expectedInstructions, const ByteCode& actual)
{
	auto flattened = ConcatInstructions(expectedInstructions);
	return TestInstructions(flattened, actual.Instructions) && TestConstants(expectedConstants, actual.Constants);
}

bool CompilerTest(const std::vector<ConstantValue>& expectedConstants, const std::vector<Instructions>& expectedInstructions, std::string input)
{
	RogueSyntax syn;
	auto byteCode = syn.Compile(input);
	return TestByteCode(expectedConstants, expectedInstructions, byteCode);
}

bool VmTest(std::string input, ConstantValue expected)
{
	RogueSyntax syn;
	auto byteCode = syn.Compile(input);
	auto str = OpCode::PrintInstructions(byteCode.Instructions);
	auto vm = syn.MakeVM(byteCode);

	try
	{
		vm->Run();
		auto actual = vm->LastPoppped();
		REQUIRE(actual != nullptr);
		return TestConstant(expected, actual);
	}
	catch (const std::exception& e)
	{
		if (std::holds_alternative<std::string>(expected))
		{
			auto expectedError = std::get<std::string>(expected);
			REQUIRE(expectedError == e.what());
			return true;
		}
		else
		{
			throw;
		}
	}
}