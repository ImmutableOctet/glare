#include <catch2/catch_test_macros.hpp>

#include <util/json.hpp>

#include <engine/meta/serial.hpp>
#include <engine/meta/meta.hpp>

#include <engine/components/name_component.hpp>

TEST_CASE("engine::load", "[engine:meta]")
{
	engine::reflect_all();

	auto data = util::json::parse("{\"name\": \"Test Name\"}");

	SECTION("Load new object")
	{
		auto name = engine::load<engine::NameComponent>(data);

		REQUIRE(name.get_name() == "Test Name");
	}

	SECTION("Load with existing object, field assignment disabled")
	{
		auto name = engine::NameComponent{ "Original Name" };

		engine::load(name, data, false);

		REQUIRE(name.get_name() == "Test Name");
	}

	SECTION("Load with existing object, field assignment enabled")
	{
		auto name = engine::NameComponent { "Original Name" };

		engine::load(name, data, true);

		REQUIRE(name.get_name() == "Test Name");
	}
}