#include <catch2/catch_test_macros.hpp>
#include <util/algorithm.hpp>

#include <unordered_map>
#include <string>
#include <tuple>

TEST_CASE("util::find_any", "[util:algorithm]")
{
	auto dict = std::unordered_map<std::string, std::string>{};

	dict["Entry A"] = "A";
	dict["Entry B"] = "B";
	dict["Entry C"] = "C";
	dict["Entry D"] = "D";

	auto str = std::string("A::B->C.D");

	SECTION("Find any in map type")
	{
		auto result = util::find_any(dict, "Entry B", "Entry C", "Entry D");

		REQUIRE(result == dict.find("Entry B"));
		REQUIRE(result->second == "B");
	}

	SECTION("Find any in map type (Extended result)")
	{
		auto result = util::find_any_ex(dict, dict.end(), "Entry C", "Entry D");

		REQUIRE(std::get<0>(result) == dict.find("Entry C"));
		REQUIRE(std::get<1>(result) == "Entry C");
		REQUIRE(std::get<0>(result)->second == "C");
	}

	SECTION("Find any in string type")
	{
		auto result = util::find_any(str, ".", "->");

		REQUIRE(result == 7);
		REQUIRE(str[result] == '.');
	}

	SECTION("Find any in string type (Extended result)")
	{
		auto result = util::find_any_ex(str, std::string::npos, "::", "->");

		REQUIRE(std::get<0>(result) == 1);
		REQUIRE(std::get<1>(result) == "::");
	}
}

TEST_CASE("util::find_first_of", "[util:algorithm]")
{
	const auto str = std::string_view("A.B::C::D.E");

	SECTION("Find first")
	{
		auto result = util::find_first_of(str, "::", ".");

		REQUIRE(result == 1);
	}

	SECTION("Find last")
	{
		auto result = util::find_last_of(str, "::", ".");

		REQUIRE(result == 9);
	}

	SECTION("Find furthest")
	{
		auto result = util::find_furthest(str, "::", ".");

		REQUIRE(result == 3);
	}
}