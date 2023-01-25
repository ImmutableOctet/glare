#include <catch2/catch_test_macros.hpp>

#include <util/json.hpp>
#include <engine/meta/serial.hpp>
#include <engine/meta/meta.hpp>

#include <engine/components/name_component.hpp>

TEST_CASE("engine::load", "[engine:meta]")
{
	using namespace nlohmann::literals;

	engine::reflect_all();

	util::json data = "{\"name\": \"Test Name\"}"_json;

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