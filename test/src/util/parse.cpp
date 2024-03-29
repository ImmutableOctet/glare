#include <catch2/catch_test_macros.hpp>

#include <string_view>

#include <util/string.hpp>
#include <util/parse.hpp>
#include <util/algorithm.hpp>

#include <array>
#include <utility>

TEST_CASE("util::find_accessor", "[util:parse]")
{
	REQUIRE(std::get<1>(util::find_accessor("A::B")) == "::");
	REQUIRE(std::get<1>(util::find_accessor("A.B")) == ".");
	REQUIRE(std::get<1>(util::find_accessor("A->B")) == "->");
	REQUIRE(std::get<0>(util::find_accessor("A::B::C")) == 1);
	REQUIRE(std::get<0>(util::find_accessor("First::Second")) == 5);
	REQUIRE(std::get<0>(util::find_accessor("First.Second::Third")) == 5);
}

TEST_CASE("util::find_last_accessor", "[util:parse]")
{
	REQUIRE(std::get<1>(util::find_last_accessor("A::B")) == "::");
	REQUIRE(std::get<1>(util::find_last_accessor("A.B")) == ".");
	REQUIRE(std::get<1>(util::find_last_accessor("A->B")) == "->");
	REQUIRE(std::get<0>(util::find_last_accessor("A::B::C")) == 4);
	REQUIRE(std::get<0>(util::find_last_accessor("First.Second::Third")) == 12);
	REQUIRE(std::get<0>(util::find_last_accessor("First.Second::Third::Fourth")) == 19);
}

TEST_CASE("util::find_logic_operator", "[util:parse]")
{
	REQUIRE(std::get<0>(util::find_logic_operator("X || Y")) == 2);
	REQUIRE(std::get<0>(util::find_logic_operator("X::value && Y::value")) == 9);
	
	REQUIRE(std::get<0>(util::find_logic_operator("X ^ some_value", true)) == 2);
	REQUIRE(std::get<0>(util::find_logic_operator("X ^ Y", false)) == std::string_view::npos);

	REQUIRE(std::get<0>(util::find_logic_operator("(X ^ some_value)", true)) == std::string_view::npos);
	REQUIRE(std::get<0>(util::find_logic_operator("(X ^ Y)", false)) == std::string_view::npos);
}

TEST_CASE("util::find_operator", "[util:parse]")
{
	SECTION("Ensure earliest operator")
	{
		REQUIRE(std::get<1>(util::find_operator("+=-||")) == "+=");
		REQUIRE(std::get<1>(util::find_operator("|||")) == "||");
		REQUIRE(std::get<1>(util::find_operator("||&&")) == "||");
		REQUIRE(std::get<1>(util::find_operator("/&&")) == "/");
	}

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

TEST_CASE("util::find_assignment_operator", "[util:parse]")
{
	SECTION("Regular assignment operator")
	{
		REQUIRE(std::get<0>(util::find_assignment_operator("x = 10", false, false, false)) == 2);
		REQUIRE(std::get<0>(util::find_assignment_operator(" y  = 50", false, false, false)) == 4);
		REQUIRE(std::get<0>(util::find_assignment_operator(" z() = 100", false, true, false)) == 5);
		REQUIRE(std::get<0>(util::find_assignment_operator(" w() = 500", false, true, true)) == std::string_view::npos);
		REQUIRE(std::get<0>(util::find_assignment_operator(" fn(a=b)", false, false, false)) == 5);
		REQUIRE(std::get<0>(util::find_assignment_operator("other_fn(a=b)", false, true, true)) == std::string_view::npos);
		REQUIRE(std::get<0>(util::find_assignment_operator("other_fn(a=b)", false, false, false)) == 10);
		REQUIRE(std::get<0>(util::find_assignment_operator("other_fn(a=b)", false, true, false)) == std::string_view::npos);
		REQUIRE(std::get<0>(util::find_assignment_operator("c=(a)", false, true, true)) == 1);

		REQUIRE(std::get<1>(util::find_assignment_operator("x = y", false, false, false)) == "=");
	}

	SECTION("Compound assignment operator")
	{
		auto [index, symbol] = util::find_assignment_operator("x += y", true, false, false);

		REQUIRE(index == 2);
		REQUIRE(symbol == "+=");
	}

	SECTION("Compound assignment operator + embedded assignment in unrelated scope")
	{
		REQUIRE(std::get<0>(util::find_assignment_operator("x += (y+=z)", true, true, false)) == 2);
		REQUIRE(std::get<0>(util::find_assignment_operator("x(y+=z)", true, true, false)) == std::string_view::npos);
		REQUIRE(std::get<0>(util::find_assignment_operator("x(y+=z)", true, false, false)) == 3);
		REQUIRE(std::get<0>(util::find_assignment_operator("x(y+=z) += 10", true, true, false)) == 8);
		REQUIRE(std::get<0>(util::find_assignment_operator("x(y+=z) += 10", true, true, true)) == std::string_view::npos);
	}
}

TEST_CASE("util::parse_variable_declaration", "[util:parse]")
{
	SECTION("Local variable, no type, no assignment + unrelated trailing")
	{
		auto
		[
			scope_qualifier,
			variable_name,
			variable_type,
			assignment_expr,
			trailing_expr
		] = util::parse_variable_declaration(std::string_view("local x unrelated"));

		REQUIRE(scope_qualifier == "local");
		REQUIRE(variable_name == "x");
		REQUIRE(variable_type.empty());
		REQUIRE(assignment_expr.empty());
		REQUIRE(trailing_expr == "unrelated");
	}

	SECTION("Local variable with type, no assignment")
	{
		auto
		[
			scope_qualifier,
			variable_name,
			variable_type,
			assignment_expr,
			trailing_expr
		] = util::parse_variable_declaration(std::string_view("local x:int"));

		REQUIRE(scope_qualifier == "local");
		REQUIRE(variable_name == "x");
		REQUIRE(variable_type == "int");
		REQUIRE(assignment_expr.empty());
		REQUIRE(trailing_expr.empty());
	}

	SECTION("Local variable, no type, no assignment")
	{
		auto
		[
			scope_qualifier,
			variable_name,
			variable_type,
			assignment_expr,
			trailing_expr
		] = util::parse_variable_declaration(std::string_view("local x"));

		REQUIRE(scope_qualifier == "local");
		REQUIRE(variable_name == "x");
		REQUIRE(variable_type.empty());
		REQUIRE(assignment_expr.empty());
		REQUIRE(trailing_expr.empty());
	}

	SECTION("Local variable with type, no assignment + unrelated trailing")
	{
		auto
		[
			scope_qualifier,
			variable_name,
			variable_type,
			assignment_expr,
			trailing_expr
		] = util::parse_variable_declaration(std::string_view("local x:int something unrelated"));

		REQUIRE(scope_qualifier == "local");
		REQUIRE(variable_name == "x");
		REQUIRE(variable_type == "int");
		REQUIRE(assignment_expr.empty());
		REQUIRE(trailing_expr == "something unrelated");
	}

	SECTION("Local variable, no assignment, no type + trailing content")
	{
		auto
		[
			scope_qualifier,
			variable_name,
			variable_type,
			assignment_expr,
			trailing_expr
		] = util::parse_variable_declaration(std::string_view("local my_var_name unrelated trailing content"));

		REQUIRE(scope_qualifier == "local");
		REQUIRE(variable_name == "my_var_name");
		REQUIRE(variable_type.empty());
		REQUIRE(assignment_expr.empty());
		REQUIRE(trailing_expr == "unrelated trailing content");
	}

	SECTION("Context variable, no type + assignment")
	{
		auto
		[
			scope_qualifier,
			variable_name,
			variable_type,
			assignment_expr,
			trailing_expr
		] = util::parse_variable_declaration(std::string_view("context context_var =     30.02 "));

		REQUIRE(scope_qualifier == "context");
		REQUIRE(variable_name == "context_var");
		REQUIRE(variable_type.empty());
		REQUIRE(assignment_expr == "30.02");
		REQUIRE(trailing_expr.empty());
	}

	SECTION("Local variable, infer type syntax")
	{
		auto
		[
			scope_qualifier,
			variable_name,
			variable_type,
			assignment_expr,
			trailing_expr
		] = util::parse_variable_declaration(std::string_view("local local_var:=     something "));

		REQUIRE(scope_qualifier == "local");
		REQUIRE(variable_name == "local_var");
		REQUIRE(variable_type.empty());
		REQUIRE(assignment_expr == "something");
		REQUIRE(trailing_expr.empty());
	}

	SECTION("Local variable + advanced assignment")
	{
		auto
		[
			scope_qualifier,
			variable_name,
			variable_type,
			assignment_expr,
			trailing_expr
		]
		= util::parse_variable_declaration(std::string_view("auto cursor_destination = (CameraSystem::get_world_space_position_from_screen_coordinates(self.Transform2DComponent::position).normalize()):Vector"));

		REQUIRE(scope_qualifier == "auto");
		REQUIRE(variable_name == "cursor_destination");
		REQUIRE(variable_type.empty());
		REQUIRE(assignment_expr == "(CameraSystem::get_world_space_position_from_screen_coordinates(self.Transform2DComponent::position).normalize()):Vector");
		REQUIRE(trailing_expr.empty());
	}

	SECTION("Global variable + type + assignment")
	{
		auto
		[
			scope_qualifier,
			variable_name,
			variable_type,
			assignment_expr,
			trailing_expr
		] = util::parse_variable_declaration(std::string_view("global global_var_name:int =     100 "));

		REQUIRE(scope_qualifier == "global");
		REQUIRE(variable_name == "global_var_name");
		REQUIRE(variable_type == "int");
		REQUIRE(assignment_expr == "100");
		REQUIRE(trailing_expr.empty());
	}
}

TEST_CASE("util::parse_key_expr_and_value_type", "[util:parse]")
{
	SECTION("Quoted key, no value-type specifier, no trailing expression")
	{
		auto [key_name, value_type, trailing_expr, expression_syntax_used] = util::parse_key_expr_and_value_type
		(
			"\"Some Key\"",
			":", false, true, true
		);

		REQUIRE(key_name == "Some Key");
		REQUIRE(value_type.empty());
		REQUIRE(trailing_expr.empty());
		REQUIRE(!expression_syntax_used);
	}

	SECTION("Quoted key with value-type specifier + trailing expression")
	{
		auto [key_name, value_type, trailing_expr, expression_syntax_used] = util::parse_key_expr_and_value_type
		(
			"\"Some Key\":some_type unrelated",
			":", true, true, false
		);

		REQUIRE(key_name == "\"Some Key\"");
		REQUIRE(value_type == "some_type");
		REQUIRE(trailing_expr == "unrelated");
		REQUIRE(!expression_syntax_used);
	}

	SECTION("Truncated key, no value-type specifier + trailing expression")
	{
		auto [key_name, value_type, trailing_expr, expression_syntax_used] = util::parse_key_expr_and_value_type
		(
			"some::key::name unrelated_expression",
			":", true, true, true, true
		);

		REQUIRE(key_name == "some::key::name");
		REQUIRE(value_type.empty());
		REQUIRE(trailing_expr == "unrelated_expression");
		REQUIRE(!expression_syntax_used);
	}

	SECTION("Regular key with value-type specifier")
	{
		auto [key_name, value_type, trailing_expr, expression_syntax_used] = util::parse_key_expr_and_value_type
		(
			"some::key:some_value_type unrelated trailing expression",
			":", true
		);

		REQUIRE(key_name == "some::key");
		REQUIRE(value_type == "some_value_type");
		REQUIRE(trailing_expr == "unrelated trailing expression");
		REQUIRE(!expression_syntax_used);
	}

	SECTION("Regular key with value-type specifier, no trailing expression allowed")
	{
		auto [key_name, value_type, trailing_expr, expression_syntax_used] = util::parse_key_expr_and_value_type
		(
			"some_key:some_value_type unrelated trailing expression",
			":", false
		);

		REQUIRE(key_name.empty());
		REQUIRE(value_type.empty());
		REQUIRE(trailing_expr.empty());
		REQUIRE(!expression_syntax_used);
	}

	SECTION("Truncated key, no value-type specifier, no trailing expression allowed")
	{
		auto [key_name, value_type, trailing_expr, expression_syntax_used] = util::parse_key_expr_and_value_type
		(
			"key_name unrelated expression",
			"|", false, true, true, true
		);

		REQUIRE(key_name.empty());
		REQUIRE(value_type.empty());
		REQUIRE(trailing_expr.empty());
		REQUIRE(!expression_syntax_used);
	}


	SECTION("Command as key, no value-type specifier + trailing expression")
	{
		auto [command_as_key, value_type, trailing_expr, command_syntax_used] = util::parse_key_expr_and_value_type
		(
			"command_name() trailing",
			":", true
		);

		REQUIRE(command_as_key == "command_name()");
		REQUIRE(value_type.empty());
		REQUIRE(trailing_expr == "trailing");
		REQUIRE(command_syntax_used);
	}

	SECTION("Command as key with value-type specifier + trailing expression")
	{
		auto [command_as_key, value_type, trailing_expr, command_syntax_used] = util::parse_key_expr_and_value_type
		(
			"command_name()|intended_value_type trailing expression",
			"|", true
		);

		REQUIRE(command_as_key == "command_name()");
		REQUIRE(value_type == "intended_value_type");
		REQUIRE(trailing_expr == "trailing expression");
		REQUIRE(command_syntax_used);
	}

	SECTION("Expression in parentheses as key with value-type specifier + trailing expression")
	{
		auto [key_name_expr, value_type, trailing_expr, expression_syntax_used] = util::parse_key_expr_and_value_type
		(
			"(some_expression:something else):value_type unrelated expression",
			":", true
		);

		REQUIRE(key_name_expr == "(some_expression:something else)");
		REQUIRE(value_type == "value_type");
		REQUIRE(trailing_expr == "unrelated expression");
		REQUIRE(expression_syntax_used);
	}

	SECTION("Expression in parentheses as key, no value-type specifier + trailing expression")
	{
		auto [key_name_expr, value_type, trailing_expr, expression_syntax_used] = util::parse_key_expr_and_value_type
		(
			"(test expression with : operator) unrelated",
			":", true
		);

		REQUIRE(key_name_expr == "(test expression with : operator)");
		REQUIRE(value_type.empty());
		REQUIRE(trailing_expr == "unrelated");
		REQUIRE(expression_syntax_used);
	}

	SECTION("Expression in parentheses as key, with value-type specifier, no trailing expression allowed")
	{
		auto [key_name_expr, value_type, trailing_expr, expression_syntax_used] = util::parse_key_expr_and_value_type
		(
			"(test expression with : operator):type unrelated",
			":", false
		);

		REQUIRE(key_name_expr.empty());
		REQUIRE(value_type.empty());
		REQUIRE(trailing_expr.empty());
		REQUIRE(!expression_syntax_used);
	}
}

TEST_CASE("util::parse_command", "[util:parse]")
{
	SECTION("Keep string content quoted")
	{
		auto command_expr = std::string_view("command_name(\"Quoted string content\")");

		auto [command_name, command_content, trailing_expr, is_string_content, parsed_length] = util::parse_command
		(
			command_expr,
			true, false, false,

			// Ensure quotes are preserved.
			false
		);

		REQUIRE(command_name == "command_name");
		REQUIRE(!command_content.empty());
		REQUIRE(util::is_quoted(command_content));
		REQUIRE(trailing_expr.empty());
		REQUIRE(is_string_content);
		REQUIRE(parsed_length == command_expr.length());
	}

	SECTION("`if` condition treated as command")
	{
		auto [command_name, command_content, trailing_expr, is_string_content, parsed_length] = util::parse_command
		(
			std::string_view("if (NameComponent::name() != \"Hello world\") then"),
			true
		);

		REQUIRE(command_name == "if"); // The 'command' is `if`.
		REQUIRE(!command_content.empty()); // Content exists.
		REQUIRE(trailing_expr == "then"); // No whitespace.
		REQUIRE(!is_string_content); // Despite having a string inside of it, the whole thing isn't a string, and is thus not considered 'string content'.
	}

	SECTION("Command with parentheses in explicit string content")
	{
		auto [command_name, command_content, trailing_expr, is_string_content, parsed_length] = util::parse_command
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
		auto [command_name, command_content, trailing_expr, is_string_content, parsed_length] = util::parse_command
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
		auto [command_name, command_content, trailing_expr, is_string_content, parsed_length] = util::parse_command
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

TEST_CASE("util::parse_command_or_value", "[util:parse]")
{
	SECTION("Truncate at operator")
	{
		auto expr = std::string_view("button == Button::Jump || OnButtonPressed::button == Button::Shield");

		auto
		[
			value, content,
			trailing_expr,
			is_string_content, is_command,
			parsed_length
		] = util::parse_command_or_value
		(
			expr,
			true, true, true, true,
			false,
			true
		);

		REQUIRE(value == "button");
		REQUIRE(content == value);
		REQUIRE(trailing_expr == "== Button::Jump || OnButtonPressed::button == Button::Shield");
		REQUIRE(!is_string_content);
		REQUIRE(!is_command);
		REQUIRE(parsed_length == value.length()+(sizeof(" ")-1));
	}

	SECTION("Empty command")
	{
		auto
		[
			command_name, content,
			trailing_expr,
			is_string_content, is_command,
			parsed_length
		] = util::parse_command_or_value(std::string_view("empty_command()"), false, true);

		REQUIRE(!command_name.empty());
		REQUIRE(command_name == "empty_command");
		REQUIRE(content.empty());
		REQUIRE(trailing_expr.empty());
		REQUIRE(is_command);
	}

	SECTION("Regular value")
	{
		auto
		[
			value_or_command, value,
			trailing_expr,
			is_string_content, is_command,
			parsed_length
		] = util::parse_command_or_value(std::string_view("some_value"));

		REQUIRE(!value_or_command.empty());
		REQUIRE(!value.empty());
		REQUIRE(trailing_expr.empty());
		REQUIRE(!is_command);
		REQUIRE(value == "some_value");
	}

	SECTION("Regular value + trailing expression")
	{
		auto
		[
			value_or_command, value,
			trailing_expr,
			is_string_content, is_command,
			parsed_length
		] = util::parse_command_or_value(std::string_view("some_value some_trailing_value"));

		REQUIRE(!value_or_command.empty());
		REQUIRE(!value.empty());
		REQUIRE(!trailing_expr.empty());
		REQUIRE(!is_command);
		REQUIRE(value == "some_value");
		REQUIRE(trailing_expr == "some_trailing_value");
	}

	SECTION("Regular value as string")
	{
		auto
		[
			value_or_command, value,
			trailing_expr,
			is_string_content, is_command,
			parsed_length
		] = util::parse_command_or_value(std::string_view("\"This is a string.\""));

		REQUIRE(!value_or_command.empty());
		REQUIRE(!value.empty());
		REQUIRE(trailing_expr.empty());
		REQUIRE(is_string_content);
		REQUIRE(!is_command);
		REQUIRE(value == "This is a string.");
	}

	SECTION("Unrelated expression expected as value")
	{
		auto
		[
			value_or_command, value,
			trailing_expr,
			is_string_content, is_command,
			parsed_length
		] = util::parse_command_or_value(std::string_view("child(fn(player_model())) .AnimationComponent::time 12345"));

		REQUIRE(value_or_command == "child");
		REQUIRE(value == "fn(player_model())");
		REQUIRE(trailing_expr == ".AnimationComponent::time 12345");
		REQUIRE(!is_string_content);
		REQUIRE(is_command);
	}

	SECTION("Value and assignment operator (+ trailing unintended accessor symbol)")
	{
		auto
		[
			value_or_command, value,
			trailing_expr,
			is_string_content, is_command,
			parsed_length
		] = util::parse_command_or_value
		(
			std::string_view("time += 0.3 * 4"),
			true,
			false,
			true, // truncate_value_at_first_accessor
			false,
			false,
			true  // truncate_value_at_operator_symbol
		);

		REQUIRE(!value_or_command.empty());
		REQUIRE(value_or_command == value);
		REQUIRE(value == "time");
		REQUIRE(trailing_expr == "+= 0.3 * 4");
		REQUIRE(!is_string_content);
		REQUIRE(!is_command);

		// Length of `time` + whitespace character.
		REQUIRE(parsed_length == (value.length() + 1));
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
	SECTION("Casted sub-expressions")
	{
		const auto expr = std::string_view("(OnMouseMove::x):string + \", \" + (OnMouseMove::y):string");
		const auto [begin, end] = util::find_parentheses(expr);

		REQUIRE(begin == 0);
		REQUIRE(end == 15);

		const auto view = expr.substr(begin, (end - begin + 1));

		REQUIRE(view == "(OnMouseMove::x)");
	}

	SECTION("Regular scope traversal")
	{
		const auto expr = std::string_view("fn((2), ((3)), 4) + other_fn(1, 2, (3)) + something_else()");
		const auto [begin, end] = util::find_parentheses(expr);

		REQUIRE(begin == 2);
		REQUIRE(end == 16);

		auto view = expr.substr(begin, (end-begin+1));

		REQUIRE(view == "((2), ((3)), 4)");
	}
}

TEST_CASE("util::parse_member_reference", "[util:parse]")
{
	SECTION("Command with member reference inside + trailing content")
	{
		auto expr = std::string_view("sleep(child(player_model).AnimationComponent::time).value::some_other_field");

		auto [leading_expr, trailing_expr, parsed_length] = util::parse_member_reference
		(
			expr, true, true
		);

		REQUIRE(leading_expr == "sleep(child(player_model).AnimationComponent::time)");
		REQUIRE(trailing_expr == "value");
		REQUIRE(parsed_length == (leading_expr.length() + (sizeof(".")-1) + trailing_expr.length()));
	}

	SECTION("Leading + member-assignment")
	{
		auto [leading_expr, trailing_expr, parsed_length] = util::parse_member_reference
		(
			std::string_view("my_type::my_member = some_value"),
			false, false, true, false, false, false
		);

		REQUIRE(leading_expr == "my_type");
		REQUIRE(trailing_expr == "my_member = some_value");
		REQUIRE(parsed_length == leading_expr.length());
	}

	SECTION("Truncate at first member")
	{
		auto [leading_expr, trailing_expr, parsed_length] = util::parse_member_reference
		(
			std::string_view("sleep(child(player_model).AnimationComponent::time)::value::something else unrelated"),
			true, true, false, true
		);

		REQUIRE(leading_expr == "sleep(child(player_model).AnimationComponent::time)");
		REQUIRE(trailing_expr == "value");
		REQUIRE(parsed_length == (leading_expr.length() + (sizeof("::")-1) + trailing_expr.length()));
	}

	SECTION("Member from member-assignment")
	{
		auto [leading_expr, trailing_expr, parsed_length] = util::parse_member_reference
		(
			std::string_view("my_type::my_member = some_value")
		);

		REQUIRE(leading_expr == "my_type");
		REQUIRE(trailing_expr == "my_member");
		REQUIRE(parsed_length == (leading_expr.length() + (sizeof("::")-1) + trailing_expr.length()));
	}

	SECTION("Regular access with `::` operator")
	{
		auto expr = std::string_view("A::B::C");
		
		auto [type_name, data_member_name, parsed_length] = util::parse_member_reference(expr);

		REQUIRE(type_name == "A::B");
		REQUIRE(data_member_name == "C");
		REQUIRE(parsed_length == expr.length());
	}

	SECTION("Extra scope in command syntax")
	{
		auto expr = std::string_view("A::B(some::embedded::value)::C");
		
		auto [type_name, data_member_name, parsed_length] = util::parse_member_reference(expr, true);

		REQUIRE(type_name == "A::B(some::embedded::value)");
		REQUIRE(data_member_name == "C");
		REQUIRE(parsed_length == expr.length());
	}

	SECTION("Command syntax")
	{
		auto expr = std::string_view("A(some text).B");

		auto [type_name, data_member_name, parsed_length] = util::parse_member_reference(expr, true);

		REQUIRE(type_name == "A(some text)");
		REQUIRE(data_member_name == "B");
		REQUIRE(parsed_length == expr.length());
	}

	SECTION("Command syntax where command syntax is prohibited")
	{
		auto [type_name, data_member_name, parsed_length] = util::parse_member_reference(std::string_view("A(some text)->B"), false);

		REQUIRE(type_name.empty());
		REQUIRE(data_member_name.empty());
		REQUIRE(parsed_length == 0);
	}

	SECTION("Unrelated command with member reference inside")
	{
		auto [leading_expr, trailing_expr, parsed_length] = util::parse_member_reference
		(
			std::string_view("sleep(child(player_model).AnimationComponent::time)"),
			true, true
		);

		REQUIRE(leading_expr.empty());
		REQUIRE(trailing_expr.empty());
		REQUIRE(parsed_length == 0);
	}

	SECTION("Allow trailing command")
	{
		auto [leading_expr, trailing_expr, parsed_length] = util::parse_member_reference
		(
			std::string_view("sleep(child(player_model).AnimationComponent::time)::value::some_method(xyz)"),
			true, true, true, false
		);

		REQUIRE(leading_expr == "sleep(child(player_model).AnimationComponent::time)");
		REQUIRE(trailing_expr == "value::some_method(xyz)");
		REQUIRE(parsed_length == (leading_expr.length() + (sizeof("::")-1) + trailing_expr.length()));
	}

	SECTION("Disallow trailing command")
	{
		auto [leading_expr, trailing_expr, parsed_length] = util::parse_member_reference
		(
			std::string_view("sleep(child(player_model).AnimationComponent::time)::value::some_method(xyz)"),
			true, true, false, false
		);

		REQUIRE(leading_expr.empty());
		REQUIRE(trailing_expr.empty());
		REQUIRE(parsed_length == 0);
	}

	SECTION("Truncate at first command in place of member")
	{
		auto [leading_expr, trailing_expr, parsed_length] = util::parse_member_reference
		(
			std::string_view("sleep(child(player_model).AnimationComponent::time)::get_value()::something else unrelated"),
			true, true, true, true
		);

		REQUIRE(leading_expr == "sleep(child(player_model).AnimationComponent::time)");
		REQUIRE(trailing_expr == "get_value()");
		REQUIRE(parsed_length == (leading_expr.length() + (sizeof("::")-1) + trailing_expr.length()));
	}

	SECTION("Abort on operator in member reference")
	{
		auto [leading_expr, trailing_expr, parsed_length] = util::parse_member_reference
		(
			std::string_view("first expr = second expr"),
			true, true, true, false, true, true
		);

		REQUIRE(leading_expr.empty());
		REQUIRE(trailing_expr.empty());
		REQUIRE(parsed_length == 0);
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

TEST_CASE("util::enumerate_unscoped_substrings")
{
	SECTION("Prefix and suffix")
	{
		auto result = std::string {};

		util::enumerate_unscoped_substrings
		(
			std::string_view { "A(\"\", test)()()(())[(test)]B" },

			std::array
			{
				std::pair<std::string_view, std::string_view> { "(", ")" },
				std::pair<std::string_view, std::string_view> { "[", "]" },
				std::pair<std::string_view, std::string_view> { "\"", "\"" }
			},

			[&result](std::string_view substr)
			{
				result += substr;
			}
		);

		REQUIRE(result == "AB");
	}

	SECTION("Unclosed scope")
	{
		auto result = std::string {};

		util::enumerate_unscoped_substrings
		(
			std::string_view{ "A(\"\"[])[B" },

			std::array
			{
				std::pair<std::string_view, std::string_view> { "(", ")" },
				std::pair<std::string_view, std::string_view> { "[", "]" },
				std::pair<std::string_view, std::string_view> { "\"", "\"" }
			},

			[&result](std::string_view substr)
			{
				result += substr;
			}
		);

		REQUIRE(result == "A[B");
	}

	SECTION("Escape characters")
	{
		auto result = std::string {};

		util::enumerate_unscoped_substrings
		(
			std::string_view{ "A\\(\\\"\\\"\\)B" },

			std::array
			{
				std::pair<std::string_view, std::string_view> { "(", ")" },
				std::pair<std::string_view, std::string_view> { "[", "]" },
				std::pair<std::string_view, std::string_view> { "\"", "\"" }
			},

			[&result](std::string_view substr)
			{
				result += substr;
			}
		);

		REQUIRE(result == "A\\(\\\"\\\"\\)B");
	}

	SECTION("Escape characters + unclosed scope")
	{
		auto result = std::string {};

		util::enumerate_unscoped_substrings
		(
			std::string_view{ "A\\(\"\"[]\\[\\]\\)[B" },

			std::array
			{
				std::pair<std::string_view, std::string_view> { "(", ")" },
				std::pair<std::string_view, std::string_view> { "[", "]" },
				std::pair<std::string_view, std::string_view> { "\"", "\"" }
			},

			[&result](std::string_view substr)
			{
				result += substr;
			}
		);

		REQUIRE(result == "A\\(\\[\\]\\)[B");
	}
}

TEST_CASE("util::find_unscoped")
{
	REQUIRE(util::find_unscoped("A(,),B", ",") == 4);
	REQUIRE(util::find_unscoped("A\\(,),B", ",") == 3);
	REQUIRE(util::find_unscoped("A(\\,) ,B", ",", 4) == 6);

	SECTION("Standard symbols + custom find function")
	{
		REQUIRE
		(
			util::find_unscoped
			(
				"A[\\,]\\||B",
				
				"|", 0,
				
				util::standard_scope_symbols,

				[](const auto& unscoped_substr, const auto& target_symbol)
				{
					return util::find_unescaped(unscoped_substr, target_symbol);
				}
			) == 7
		);
	}

	SECTION("Custom symbols + custom find function + offset")
	{
		const auto custom_symbols = std::array
		{
			std::pair<std::string_view, std::string_view> { "{", "}" }
		};

		REQUIRE
		(
			util::find_unscoped
			(
				",A{\\,}\\, ,B",
				
				",", 1,
				
				custom_symbols,

				[](const auto& unscoped_substr, const auto& target_symbol)
				{
					return util::find_unescaped(unscoped_substr, target_symbol);
				}
			) == 9
		);
	}
}