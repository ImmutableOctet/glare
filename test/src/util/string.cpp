#include <catch2/catch_test_macros.hpp>
#include <util/string.hpp>

#include <cstdint>
TEST_CASE("util::is_quoted", "[util:string]")
{
	REQUIRE(!util::is_quoted(std::string_view("\"Test\" == \"Test\"")));
	REQUIRE(!util::is_quoted(std::string_view("\"Test\\\" == \\\"Test\\\"")));
	REQUIRE(util::is_quoted(std::string_view("\"Test\\\" == \\\"Test\"")));
	REQUIRE(util::is_quoted(std::string_view("\"Test\"")));
	REQUIRE(!util::is_quoted(std::string_view("\\\"Test\\\"")));
	REQUIRE(!util::is_quoted(std::string_view("\"Test\\\"")));
	REQUIRE(!util::is_quoted(std::string_view("Test")));
}

TEST_CASE("util::is_whitespace", "[util:string]")
{
	REQUIRE(util::is_whitespace(' '));
	REQUIRE(!util::is_whitespace('A'));
	REQUIRE(util::is_whitespace("     \t\t\n  \t\n"));
	REQUIRE(!util::is_whitespace("    \t \n  \t\tabc\t\n"));
	REQUIRE(!util::is_whitespace("Nowhitespacecharactersatall"));
}

TEST_CASE("util::trim", "[util:string]")
{
	REQUIRE(util::trim_beginning(" Test ") == "Test ");
	REQUIRE(util::trim_ending(" Test ") == " Test");
	REQUIRE(util::trim(" Hello world ") == "Hello world");
	REQUIRE(util::trim("  ", util::whitespace_symbols, true).empty());
	REQUIRE(!util::trim("  ", util::whitespace_symbols, false).empty());
	REQUIRE(util::trim("some::values::etc") == "some::values::etc");
}

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