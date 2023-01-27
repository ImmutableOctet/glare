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

TEST_CASE("parse_single_argument_command_or_value", "[util:parse]")
{
	SECTION("Regular value")
	{
		auto
		[
			value_or_command, value,
			trailing_expr,
			is_string_content, is_command
		] = util::parse_single_argument_command_or_value(std::string_view("some_value"));

		REQUIRE(!value_or_command.empty());
		REQUIRE(!value.empty());
		REQUIRE(trailing_expr.empty());
		REQUIRE(!is_command);
		REQUIRE(value == "some_value");
	}

	SECTION("Regular value as string")
	{
		auto
		[
			value_or_command, value,
			trailing_expr,
			is_string_content, is_command
		] = util::parse_single_argument_command_or_value(std::string_view("\"This is a string.\""));

		REQUIRE(!value_or_command.empty());
		REQUIRE(!value.empty());
		REQUIRE(trailing_expr.empty());
		REQUIRE(is_string_content);
		REQUIRE(!is_command);
		REQUIRE(value == "This is a string.");
	}
}

TEST_CASE("parse_standard_operator_segment", "[util:parse]")
{
	SECTION("+ operator with trailing content")
	{
		auto [operator_symbol, trailing_content] = util::parse_standard_operator_segment(std::string_view("+ 12345"));

		REQUIRE(operator_symbol == "+");
		REQUIRE(trailing_content == "12345");
	}

	SECTION("- operator without trailing content")
	{
		auto [operator_symbol, trailing_content] = util::parse_standard_operator_segment(std::string_view("-"));

		REQUIRE(operator_symbol == "-");
		REQUIRE(trailing_content.empty());
	}

	SECTION("/ operator with illegal trailing content (Force-disabled)")
	{
		auto [operator_symbol, trailing_content] = util::parse_standard_operator_segment(std::string_view("/trailing"), false);

		REQUIRE(operator_symbol.empty());
		REQUIRE(trailing_content.empty());
	}
}