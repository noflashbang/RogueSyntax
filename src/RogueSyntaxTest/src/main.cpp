
#include <RogueSyntaxApi.h>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("RogueSyntax_Create returns a valid handle")
{
	int handle = RogueSyntax_Create();
	REQUIRE(handle != 0);
}