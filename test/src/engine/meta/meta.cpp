#include <catch2/catch_test_macros.hpp>

#include <engine/meta/meta.hpp>
#include <engine/meta/serial.hpp>

#include <engine/input/buttons.hpp>

#include <string_view>

TEST_CASE("engine:meta", "[engine]")
{
	SECTION("meta_any_from_string")
	{
		engine::reflect_all();

		{
			auto result = engine::meta_any_from_string(std::string_view("Button::Jump"));

			REQUIRE(result);
			REQUIRE(result == engine::Button::Jump);
		}

		{
			auto result = engine::meta_any_from_string(std::string_view("hash(\"Button::Jump\")"));

			REQUIRE(result);
			REQUIRE(result == engine::hash("Button::Jump").value());
		}

		{
			auto result = engine::meta_any_from_string(std::string_view("#Button::Jump"));

			REQUIRE(result);
			REQUIRE(result == engine::hash("Button::Jump").value());
		}
	}
}