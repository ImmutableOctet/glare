#include <catch2/catch_test_macros.hpp>

#include <util/json.hpp>

#include <engine/timer.hpp>
#include <engine/entity/parse.hpp>
#include <engine/meta/meta.hpp>
#include <engine/meta/hash.hpp>

#include <engine/entity/entity_target.hpp>

#include <string_view>

TEST_CASE("engine::parse_qualified_assignment_or_comparison", "[engine:entity]")
{
	SECTION("Trigger condition after logical operator")
	{
		auto expr = std::string_view(" || OnButtonPressed::button == Button::Shield");

		auto
		[
			entity_ref_expr,
			first_symbol, second_symbol,
			operator_symbol, value_raw,
			updated_offset
		] = engine::parse_qualified_assignment_or_comparison
		(
			expr, 0, {},
			true, true, true, true
		);

		REQUIRE(entity_ref_expr.empty());
		REQUIRE(first_symbol == "OnButtonPressed");
		REQUIRE(second_symbol == "button");
		REQUIRE(operator_symbol == "==");
		REQUIRE(value_raw == "Button::Shield");
		REQUIRE(updated_offset == expr.length());
	}

	SECTION("Get first trigger condition from multiple")
	{
		auto expr = std::string_view("OnButtonPressed::button == Button::Jump || OnButtonPressed::button == Button::Shield");

		auto
		[
			entity_ref_expr,
			first_symbol, second_symbol,
			operator_symbol, value_raw,
			updated_offset
		] = engine::parse_qualified_assignment_or_comparison
		(
			expr, 0, {},
			true, true, true, true
		);

		REQUIRE(entity_ref_expr.empty());
		REQUIRE(first_symbol == "OnButtonPressed");
		REQUIRE(second_symbol == "button");
		REQUIRE(operator_symbol == "==");
		REQUIRE(value_raw == "Button::Jump");
		REQUIRE(updated_offset == (sizeof("OnButtonPressed::button == Button::Jump")-1));
	}

	SECTION("Event type + value without member")
	{
		auto expr = std::string_view("OnButtonPressed|Button::Pause");

		auto
		[
			entity_ref_expr,
			first_symbol, second_symbol,
			operator_symbol, value_raw,
			updated_offset
		] = engine::parse_qualified_assignment_or_comparison
		(
			expr, 0, {}, // 29
			true, true, true, true
		);

		REQUIRE(entity_ref_expr.empty());
		REQUIRE(first_symbol == "OnButtonPressed");
		REQUIRE(second_symbol.empty());
		REQUIRE(value_raw == "Button::Pause");
		REQUIRE(operator_symbol == "|");
	}

	SECTION("Allowed operator")
	{
		auto expr = std::string_view("x == 5");

		auto
		[
			entity_ref_expr,
			first_symbol, second_symbol,
			operator_symbol, value_raw,
			updated_offset
		] = engine::parse_qualified_assignment_or_comparison(expr, 0, "==<>");

		REQUIRE(entity_ref_expr.empty());
		REQUIRE(first_symbol == "x");
		REQUIRE(second_symbol.empty());
		REQUIRE(operator_symbol == "==");
		REQUIRE(value_raw == "5");
		REQUIRE(updated_offset == expr.length());
	}

	SECTION("Disallowed operator")
	{
		auto expr = std::string_view("x != 5");

		auto
		[
			entity_ref_expr,
			first_symbol, second_symbol,
			operator_symbol, value_raw,
			updated_offset
		] = engine::parse_qualified_assignment_or_comparison(expr, 0, "==");

		REQUIRE(entity_ref_expr.empty());
		REQUIRE(first_symbol.empty());
		REQUIRE(second_symbol.empty());
		REQUIRE(operator_symbol.empty());
		REQUIRE(value_raw.empty());
		REQUIRE(updated_offset == 0);
	}

	SECTION("Simple expression with parentheses")
	{
		auto expr = std::string_view("(x == 5)");

		auto
		[
			entity_ref_expr,
			first_symbol, second_symbol,
			operator_symbol, value_raw,
			updated_offset
		] = engine::parse_qualified_assignment_or_comparison(expr);

		REQUIRE(entity_ref_expr.empty());
		REQUIRE(first_symbol == "x");
		REQUIRE(second_symbol.empty());
		REQUIRE(operator_symbol == "==");
		REQUIRE(value_raw == "5");
		REQUIRE(updated_offset == expr.length());
	}

	SECTION("Allow for missing comparison operator")
	{
		auto expr = std::string_view("(x)");

		auto
		[
			entity_ref_expr,
			first_symbol, second_symbol,
			operator_symbol, value_raw,
			updated_offset
		] = engine::parse_qualified_assignment_or_comparison
		(
			expr, 0, {},
			true, true
		);

		REQUIRE(entity_ref_expr.empty());
		REQUIRE(first_symbol == "x");
		REQUIRE(second_symbol.empty());
		REQUIRE(operator_symbol.empty());
		REQUIRE(value_raw.empty());
		REQUIRE(updated_offset == expr.length());
	}

	SECTION("Allow for missing comparison operator + trailing value")
	{
		auto expr = std::string_view("(x 5)");

		auto
		[
			entity_ref_expr,
			first_symbol, second_symbol,
			operator_symbol, value_raw,
			updated_offset
		] = engine::parse_qualified_assignment_or_comparison
		(
			expr, 0, {},
			true, false
		);

		REQUIRE(entity_ref_expr.empty());
		REQUIRE(first_symbol == "x");
		REQUIRE(second_symbol.empty());
		REQUIRE(operator_symbol.empty());
		REQUIRE(value_raw == "5");
		REQUIRE(updated_offset == expr.length());
	}

	SECTION("Allow for missing comparison operator + no expected trailing value")
	{
		auto expr = std::string_view("(x)");

		auto
		[
			entity_ref_expr,
			first_symbol, second_symbol,
			operator_symbol, value_raw,
			updated_offset
		] = engine::parse_qualified_assignment_or_comparison
		(
			expr, 0, {},
			true, false, false
		);

		REQUIRE(entity_ref_expr.empty());
		REQUIRE(first_symbol.empty());
		REQUIRE(second_symbol.empty());
		REQUIRE(operator_symbol.empty());
		REQUIRE(value_raw.empty());
		REQUIRE(updated_offset == 0);
	}

	SECTION("Entity reference + type + parentheses")
	{
		auto expr = std::string_view("(self.SomeType==Value)");

		auto
		[
			entity_ref_expr,
			first_symbol, second_symbol,
			operator_symbol, value_raw,
			updated_offset
		] = engine::parse_qualified_assignment_or_comparison(expr);

		REQUIRE(entity_ref_expr == "self");
		REQUIRE(first_symbol == "SomeType");
		REQUIRE(second_symbol.empty());
		REQUIRE(operator_symbol == "==");
		REQUIRE(value_raw == "Value");
		REQUIRE(updated_offset == expr.length());
	}

	SECTION("Entity reference + type + member + parentheses")
	{
		auto expr = std::string_view("(self.SomeType->member)");

		auto
		[
			entity_ref_expr,
			first_symbol, second_symbol,
			operator_symbol, value_raw,
			updated_offset
		] = engine::parse_qualified_assignment_or_comparison(expr);

		REQUIRE(entity_ref_expr == "self");
		REQUIRE(first_symbol == "SomeType");
		REQUIRE(second_symbol == "member");
		REQUIRE(operator_symbol.empty());
		REQUIRE(value_raw.empty());
		REQUIRE(updated_offset == expr.length());
	}

	SECTION("Entity reference + type + separator + parentheses")
	{
		auto expr = std::string_view("(child(some_child).Type|Value)");

		auto
		[
			entity_ref_expr,
			first_symbol, second_symbol,
			operator_symbol, value_raw,
			updated_offset
		] = engine::parse_qualified_assignment_or_comparison(expr);

		REQUIRE(entity_ref_expr == "child(some_child)");
		REQUIRE(first_symbol == "Type");
		REQUIRE(second_symbol.empty());
		REQUIRE(operator_symbol == "|");
		REQUIRE(value_raw == "Value");
		REQUIRE(updated_offset == expr.length());
	}

	SECTION("`+=` operator on child entity's component member")
	{
		auto expr = std::string_view("child(player_model).AnimationComponent::time += 0.3 * 4");

		auto
		[
			entity_ref_expr,
			type_name, member_name,
			operator_symbol, value_raw,
			updated_offset
		] = engine::parse_qualified_assignment_or_comparison(expr);

		REQUIRE(entity_ref_expr == "child(player_model)");
		REQUIRE(type_name == "AnimationComponent");
		REQUIRE(member_name == "time");
		REQUIRE(operator_symbol == "+=");
		REQUIRE(value_raw == "0.3 * 4");
		REQUIRE(updated_offset == expr.length());
	}
}

TEST_CASE("engine::parse_time_duration", "[engine:entity]")
{
	SECTION("Integral string value")
	{
		auto duration = engine::parse_time_duration(std::string_view("100ms"));

		REQUIRE(duration);
		REQUIRE(*duration == engine::Timer::Milliseconds(100));
	}

	SECTION("Floating-point string value")
	{
		using namespace engine::literals;

		auto duration = engine::parse_time_duration(std::string_view("12.0ms"));

		REQUIRE(duration);
		REQUIRE(std::chrono::round<std::chrono::milliseconds>(*duration) == engine::Timer::Milliseconds(12));
	}

	SECTION("String JSON value")
	{
		auto raw_value = util::json::parse("\"1.5s\"");

		auto duration = engine::parse_time_duration(raw_value);

		REQUIRE(duration);
		REQUIRE(*duration == engine::Timer::to_duration(1.5f));
	}

	SECTION("Floating-point JSON value")
	{
		auto raw_value = util::json::parse("0.5");

		auto duration = engine::parse_time_duration(raw_value);

		REQUIRE(duration);
		REQUIRE(*duration == engine::Timer::to_duration(0.5f));
	}

	SECTION("Integral JSON value")
	{
		using namespace engine::literals;

		auto raw_value = util::json::parse("100");

		auto duration = engine::parse_time_duration(raw_value);

		REQUIRE(duration);
		REQUIRE(*duration == engine::Timer::Seconds(100));
	}
}

TEST_CASE("engine::EntityTarget::parse_type", "[engine:entity]")
{
	SECTION("Regular usage")
	{
		const auto target_expr = std::string_view("child(player_model)");
		const auto result = engine::EntityTarget::parse_type(target_expr);

		REQUIRE(result);

		const auto& [target_type, offset, is_shorthand] = *result;

		REQUIRE(offset == target_expr.length());
		REQUIRE(!is_shorthand);

		REQUIRE(target_type.index() == static_cast<std::size_t>(engine::EntityTarget::Child));

		const auto as_child = std::get_if<engine::EntityTarget::ChildTarget>(&target_type);

		REQUIRE(as_child);
		REQUIRE(as_child->child_name == engine::hash("player_model").value());
	}

	SECTION("Shorthand")
	{
		const auto target_expr = std::string_view("self");
		const auto result = engine::EntityTarget::parse_type(target_expr);

		REQUIRE(result);

		const auto& [target_type, offset, is_shorthand] = *result;

		REQUIRE(target_type.index() == static_cast<std::size_t>(engine::EntityTarget::Self));
		REQUIRE(offset == target_expr.length());
		REQUIRE(is_shorthand);
	}

	SECTION("Regular usage + trailing content truncation")
	{
		const auto target_expr = std::string_view("child(player_model).NameComponent::name");
		const auto result = engine::EntityTarget::parse_type(target_expr);

		REQUIRE(result);

		const auto& [target_type, offset, is_shorthand] = *result;

		REQUIRE(offset == std::string_view("child(player_model)").length());
		REQUIRE(!is_shorthand);

		REQUIRE(target_type.index() == static_cast<std::size_t>(engine::EntityTarget::Child));
	}

	SECTION("Shorthand + trailing content truncation")
	{
		const auto target_expr = std::string_view("self.NameComponent::name");
		const auto result = engine::EntityTarget::parse_type(target_expr);

		REQUIRE(result);

		const auto& [target_type, offset, is_shorthand] = *result;

		REQUIRE(offset == std::string_view("self").length());
		REQUIRE(is_shorthand);

		REQUIRE(target_type.index() == static_cast<std::size_t>(engine::EntityTarget::Self));
	}
}