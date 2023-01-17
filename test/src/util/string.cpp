#include <catch2/catch_test_macros.hpp>
#include <util/string.hpp>

#include <cstdint>

TEST_CASE("util:string", "[util]")
{
	SECTION("from_string")
	{
		// Empty command:
		{
			REQUIRE(util::from_string<std::int32_t>("1"));
			REQUIRE(util::from_string<float>("1.5"));
			REQUIRE(!util::from_string<std::int32_t>("1.0", true));
			REQUIRE(util::from_string<std::int32_t>("1.0", false).value_or(0) == 1);
			REQUIRE(util::from_string<std::int32_t>("1.35", false).value_or(0) == 1);
		}
	}
}