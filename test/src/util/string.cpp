#include <catch2/catch_test_macros.hpp>

#include <util/string.hpp>

#include <string>
#include <string_view>
#include <cstdint>
#include <vector>
#include <tuple>

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

TEST_CASE("util::split_from", "[util:string]")
{
	std::string_view content = "sleep_test, false";

	auto result = util::split_from<2>(content, ",", 0);

	REQUIRE(result);

	REQUIRE(std::get<0>(*result) == "sleep_test");
	REQUIRE(std::get<1>(*result) == "false");
}

TEST_CASE("util::split", "[util:string]")
{
	SECTION("Normal use-case")
	{
		std::string_view content = "Some::String.With->Separators";

		std::vector<std::string_view> result;

		util::split
		(
			content,

			std::array
			{
				std::string_view { "::" },
				std::string_view { "."  },
				std::string_view { "->" }
			},

			[&result](std::string_view symbol, bool is_last_symbol) -> bool
			{
				result.emplace_back(symbol);

				return true;
			}
		);

		REQUIRE(result.size() == 4);
		REQUIRE(result[0] == "Some");
		REQUIRE(result[1] == "String");
		REQUIRE(result[2] == "With");
		REQUIRE(result[3] == "Separators");
	}

	SECTION("Stop execution early")
	{
		std::string_view content = "First message::something else|etc";

		std::string_view first_result;

		util::split
		(
			content,

			std::array
			{
				std::string_view { "::" },
				std::string_view { "|"  }
			},

			[&first_result](std::string_view symbol, bool is_last_symbol) -> bool
			{
				first_result = symbol;

				return false;
			}
		);

		REQUIRE(!first_result.empty());
		REQUIRE(first_result == "First message");
	}

	SECTION("Empty string between separators")
	{
		std::string_view content = "X..Y::::Z";

		std::vector<std::string_view> result;

		util::split
		(
			content,

			std::array
			{
				std::string_view { "::" },
				std::string_view { "."  }
			},

			[&result](std::string_view symbol, bool is_last_symbol) -> bool
			{
				result.emplace_back(symbol);

				return true;
			}
		);

		REQUIRE(result.size() == 3);
		REQUIRE(result[0] == "X");
		REQUIRE(result[1] == "Y");
		REQUIRE(result[2] == "Z");
	}

	SECTION("String beginning with separators")
	{
		std::string_view content = "...X::Y";

		std::vector<std::string_view> result;

		util::split
		(
			content,

			std::array
			{
				std::string_view { "::" },
				std::string_view { "."  }
			},

			[&result](std::string_view symbol, bool is_last_symbol) -> bool
			{
				result.emplace_back(symbol);

				return true;
			}
		);

		REQUIRE(result.size() == 2);
		REQUIRE(result[0] == "X");
		REQUIRE(result[1] == "Y");
	}

	SECTION("Callback alters `string_view` slice")
	{
		std::string_view content = "...fn(X::Y)::Z";

		std::vector<std::string_view> result;

		util::split
		(
			content,

			std::array
			{
				std::string_view { "::" },
				std::string_view { "."  }
			},

			[&content, &result](std::string_view& symbol, bool is_last_symbol) -> bool
			{
				if (auto call_begin = symbol.find('('); call_begin != std::string_view::npos)
				{
					const auto call_position_in_content = static_cast<std::size_t>((symbol.data() + call_begin) - content.data());

					if (auto call_end_position_in_content = content.find(')', call_position_in_content); call_end_position_in_content != std::string_view::npos)
					{
						const auto symbol_position_in_content = (symbol.data() - content.data());
						const auto call_expr_length = ((call_end_position_in_content + 1) - symbol_position_in_content);

						symbol = content.substr(symbol_position_in_content, call_expr_length);
					}
				}

				result.emplace_back(symbol);

				return true;
			}
		);

		REQUIRE(result.size() == 2);
		REQUIRE(result[0] == "fn(X::Y)");
		REQUIRE(result[1] == "Z");
	}
}

TEST_CASE("util::from_string", "[util:string]")
{
	REQUIRE(util::from_string<std::int32_t>("1"));
	REQUIRE(util::from_string<float>("1.5"));
	REQUIRE(!util::from_string<std::int32_t>("1.0", true));
	REQUIRE(util::from_string<std::int32_t>("1.0", false).value_or(0) == 1);
	REQUIRE(util::from_string<std::int32_t>("1.35", false).value_or(0) == 1);
}

TEST_CASE("util::find_last_singular", "[util:parse]")
{
	REQUIRE(util::find_last_singular("::: ::: : :", ":") == 10);
	REQUIRE(util::find_last_singular("some::key::name:: unrelated_expression", ":") == std::string_view::npos);
	REQUIRE(util::find_last_singular("some::key:::name:type:", ":") == 21);
}

TEST_CASE("util::find_singular", "[util:parse]")
{
	REQUIRE(util::find_singular("::: ::: :", ":") == 8);
	REQUIRE(util::find_singular("some::key::name unrelated_expression", ":") == std::string_view::npos);
	REQUIRE(util::find_singular("some::key:::name:type", ":") == 16);
}