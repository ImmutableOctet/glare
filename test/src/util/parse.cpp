#include <catch2/catch_test_macros.hpp>

#include <string_view>

#include <util/string.hpp>
#include <util/parse.hpp>
#include <util/algorithm.hpp>

TEST_CASE("util::find_operator", "[util::parse]")
{
	SECTION("Regular operators")
	{
		REQUIRE(std::get<1>(util::find_operator("+")) == "+");
		REQUIRE(std::get<1>(util::find_operator("-")) == "-");
		REQUIRE(std::get<1>(util::find_operator("!")) == "!");
		REQUIRE(std::get<1>(util::find_operator("~")) == "~");
		REQUIRE(std::get<1>(util::find_operator("*")) == "*");
		REQUIRE(std::get<1>(util::find_operator("/")) == "/");
		REQUIRE(std::get<1>(util::find_operator("<<")) == "<<");
		REQUIRE(std::get<1>(util::find_operator(">>")) == ">>");

		REQUIRE(std::get<1>(util::find_operator("&")) == "&");
		REQUIRE(std::get<1>(util::find_operator("^")) == "^");
		REQUIRE(std::get<1>(util::find_operator("|")) == "|");
		REQUIRE(std::get<1>(util::find_operator("&&")) == "&&");
		REQUIRE(std::get<1>(util::find_operator("||")) == "||");
	}

	SECTION("Comparison operators")
	{
		REQUIRE(std::get<1>(util::find_operator("<")) == "<");
		REQUIRE(std::get<1>(util::find_operator(">")) == ">");
		REQUIRE(std::get<1>(util::find_operator("<=")) == "<=");
		REQUIRE(std::get<1>(util::find_operator(">=")) == ">=");
		REQUIRE(std::get<1>(util::find_operator("==")) == "==");
		REQUIRE(std::get<1>(util::find_operator("!=")) == "!=");
		REQUIRE(std::get<1>(util::find_operator("<>")) == "<>");
	}

	SECTION("Assignment operators")
	{
		REQUIRE(std::get<1>(util::find_operator("=")) == "=");
		REQUIRE(std::get<1>(util::find_operator("+=")) == "+=");
		REQUIRE(std::get<1>(util::find_operator("-=")) == "-=");
		REQUIRE(std::get<1>(util::find_operator("*=")) == "*=");
		REQUIRE(std::get<1>(util::find_operator("/=")) == "/=");
		REQUIRE(std::get<1>(util::find_operator("%=")) == "%=");
		REQUIRE(std::get<1>(util::find_operator("<<=")) == "<<=");
		REQUIRE(std::get<1>(util::find_operator(">>=")) == ">>=");
		REQUIRE(std::get<1>(util::find_operator("&=")) == "&=");
		REQUIRE(std::get<1>(util::find_operator("|=")) == "|=");
	}
}

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

	SECTION("Unrelated expression that uses parentheses")
	{
		auto
		[
			command_name,
			command_content,
			trailing_expr,
			is_string_content
		] = util::parse_single_argument_command(std::string_view("a + b(c)"), true, true);
		
		REQUIRE(command_name.empty());
		REQUIRE(command_content.empty());
		REQUIRE(trailing_expr.empty());
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

TEST_CASE("util::parse_standard_operator_segment", "[util:parse]")
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

TEST_CASE("util::parse_trailing_reference", "[util:parse]")
{
	SECTION("Leading + Trailing")
	{
		auto [leading, trailing, access_operator] = util::parse_trailing_reference(std::string_view("a::b"));

		REQUIRE(leading == "a");
		REQUIRE(trailing == "b");
		REQUIRE(access_operator == "::");
	}

	SECTION("Trailing only")
	{
		auto [leading, trailing, access_operator] = util::parse_trailing_reference(std::string_view("->test"));

		REQUIRE(leading.empty());
		REQUIRE(trailing == "test");
		REQUIRE(access_operator == "->");
	}

	SECTION("Leading only")
	{
		auto [leading, trailing, access_operator] = util::parse_trailing_reference(std::string_view("test"), true, true);

		REQUIRE(leading == "test");
		REQUIRE(trailing.empty());
		REQUIRE(access_operator.empty());
	}

	SECTION("Trailing + Unrelated Content")
	{
		auto [leading, trailing, access_operator] = util::parse_trailing_reference(std::string_view(".intended_value other unrelated content"), true, false, true);

		REQUIRE(leading.empty());
		REQUIRE(trailing == "intended_value");
		REQUIRE(access_operator == ".");
	}
}

TEST_CASE("util::find_parentheses", "[util:string]")
{
	const auto expr = std::string_view("fn((2), ((3)), 4) + other_fn(1, 2, (3)) + something_else()");
	const auto [begin, end] = util::find_parentheses(expr);

	REQUIRE(begin == 2);
	REQUIRE(end == 16);

	auto view = expr.substr(begin, (end-begin+1));

	REQUIRE(view == "((2), ((3)), 4)");
}

TEST_CASE("util::parse_member_reference", "[util:parse]")
{
	SECTION("Regular access with `::` operator")
	{
		auto [type_name, data_member_name] = util::parse_member_reference(std::string_view("A::B"));

		REQUIRE(type_name == "A");
		REQUIRE(data_member_name == "B");
	}

	SECTION("Command syntax")
	{
		auto [type_name, data_member_name] = util::parse_member_reference(std::string_view("A(some text).B"), true);

		REQUIRE(type_name == "A(some text)");
		REQUIRE(data_member_name == "B");
	}

	SECTION("Command syntax where command syntax is prohibited")
	{
		auto [type_name, data_member_name] = util::parse_member_reference(std::string_view("A(some text)->B"), false);

		REQUIRE(type_name.empty());
		REQUIRE(data_member_name.empty());
	}

	SECTION("Command with member reference inside")
	{
		auto [leading_expr, trailing_expr] = util::parse_member_reference
		(
			std::string_view("sleep(child(player_model).AnimationComponent::time)"),
			true, true
		);

		REQUIRE(leading_expr.empty());
		REQUIRE(trailing_expr.empty());
	}

	SECTION("Command with member reference inside + trailing content")
	{
		auto [leading_expr, trailing_expr] = util::parse_member_reference
		(
			std::string_view("sleep(child(player_model).AnimationComponent::time)::value::some_other_field"),
			true, true
		);

		REQUIRE(leading_expr == "sleep(child(player_model).AnimationComponent::time)");
		REQUIRE(trailing_expr == "value::some_other_field");
	}

	SECTION("Allow trailing command")
	{
		auto [leading_expr, trailing_expr] = util::parse_member_reference
		(
			std::string_view("sleep(child(player_model).AnimationComponent::time)::value::some_method(xyz)"),
			true, true, true, false
		);

		REQUIRE(leading_expr == "sleep(child(player_model).AnimationComponent::time)");
		REQUIRE(trailing_expr == "value::some_method(xyz)");
}

	SECTION("Disallow trailing command")
	{
		auto [leading_expr, trailing_expr] = util::parse_member_reference
		(
			std::string_view("sleep(child(player_model).AnimationComponent::time)::value::some_method(xyz)"),
			true, true, false, false
		);

		REQUIRE(leading_expr.empty());
		REQUIRE(trailing_expr.empty());
	}

	SECTION("Truncate at first member")
	{
		auto [leading_expr, trailing_expr] = util::parse_member_reference
		(
			std::string_view("sleep(child(player_model).AnimationComponent::time)::value::something else unrelated"),
			true, true, false, true
		);

		REQUIRE(leading_expr == "sleep(child(player_model).AnimationComponent::time)");
		REQUIRE(trailing_expr == "value");
	}

	SECTION("Truncate at first command in place of member")
	{
		auto [leading_expr, trailing_expr] = util::parse_member_reference
		(
			std::string_view("sleep(child(player_model).AnimationComponent::time)::get_value()::something else unrelated"),
			true, true, true, true
		);

		REQUIRE(leading_expr == "sleep(child(player_model).AnimationComponent::time)");
		REQUIRE(trailing_expr == "get_value()");
	}
}

TEST_CASE("util::find_quotes", "[util:parse]")
{
	SECTION("Embedded escaped string")
	{
		const auto test_string = std::string_view("\"Some \\\"string\\\"\"");

		auto [first_quote, second_quote] = util::find_quotes(test_string);

		REQUIRE(first_quote == 0);
		REQUIRE(second_quote == (test_string.length() - 1));
	}

	SECTION("Quote without unquote")
	{
		const auto test_string = std::string_view("\"Text without ending quote");

		auto [first_quote, second_quote] = util::find_quotes(test_string);

		REQUIRE(first_quote == std::string_view::npos);
		REQUIRE(second_quote == std::string_view::npos);
	}

	SECTION("Unquote without quote")
	{
		const auto test_string = std::string_view("Text without beginning quote\"");

		auto [first_quote, second_quote] = util::find_quotes(test_string);

		REQUIRE(first_quote == std::string_view::npos);
		REQUIRE(second_quote == std::string_view::npos);
	}

	SECTION("Only escaped quotes")
	{
		const auto test_string = std::string_view("\\\"Text without beginning quote\\\"");

		auto [first_quote, second_quote] = util::find_quotes(test_string);

		REQUIRE(first_quote == std::string_view::npos);
		REQUIRE(second_quote == std::string_view::npos);
	}

	SECTION("Beginning quote with only escaped quotes")
	{
		const auto test_string = std::string_view("\"Text without beginning quote\\\"");

		auto [first_quote, second_quote] = util::find_quotes(test_string);

		REQUIRE(first_quote == std::string_view::npos);
		REQUIRE(second_quote == std::string_view::npos);
	}
}