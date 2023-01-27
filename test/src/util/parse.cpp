#include <catch2/catch_test_macros.hpp>
#include <util/parse.hpp>

#include <string_view>

TEST_CASE("parse_data_member_reference", "[util:parse]")
{
	SECTION("Regular access with `::` operator")
	{
		auto [type_name, data_member_name] = util::parse_data_member_reference(std::string_view("A::B"));

		REQUIRE(type_name == "A");
		REQUIRE(data_member_name == "B");
	}

	SECTION("Command syntax")
	{
		auto [type_name, data_member_name] = util::parse_data_member_reference(std::string_view("A(some text).B"), true);

		REQUIRE(type_name == "A(some text)");
		REQUIRE(data_member_name == "B");
	}

	SECTION("Command syntax where command syntax is prohibited")
	{
		auto [type_name, data_member_name] = util::parse_data_member_reference(std::string_view("A(some text)->B"), false);

		REQUIRE(type_name.empty());
		REQUIRE(data_member_name.empty());
	}
}

TEST_CASE("parse_single_argument_command", "[util:parse]")
{
	SECTION("`if` condition treated as command")
	{
		auto [command_name, command_content, trailing_expr, is_string_content] = util::parse_single_argument_command
		(
			std::string_view("if (NameComponent::name != \"Hello world\") then"),
			true
		);

		REQUIRE(command_name == "if"); // The 'command' is `if`.
		REQUIRE(!command_content.empty()); // Content exists.
		REQUIRE(trailing_expr == "then"); // No whitespace.
		REQUIRE(!is_string_content); // Despite having a string inside of it, the whole thing isn't a string, and is thus not considered 'string content'.
	}

	SECTION("Command with parentheses in explicit string content")
	{
		auto [command_name, command_content, trailing_expr, is_string_content] = util::parse_single_argument_command
		(
			std::string_view("function_name(\"some)()_value\") trailing_expr"),
			true
		);

		REQUIRE(command_name == "function_name");
		REQUIRE(command_content == "some)()_value");
		REQUIRE(trailing_expr == "trailing_expr");
		REQUIRE(is_string_content);
	}

	SECTION("Standard yield instruction")
	{
		auto [command_name, command_content, trailing_expr, is_string_content] = util::parse_single_argument_command
		(
			std::string_view("yield(OnButtonPressed::button|Button::Jump)"),
			true
		);

		REQUIRE(command_name == "yield");
		REQUIRE(command_content == "OnButtonPressed::button|Button::Jump");
		REQUIRE(trailing_expr.empty());
		REQUIRE(!is_string_content);
	}

	SECTION("Empty command")
	{
		auto [command_name, command_content, trailing_expr, is_string_content] = util::parse_single_argument_command
		(
			std::string_view("pause()"),
			true
		);

		REQUIRE(command_name == "pause");
		REQUIRE(command_content.empty());
		REQUIRE(trailing_expr.empty());
		REQUIRE(!is_string_content);
	}
}