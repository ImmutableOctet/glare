#include <catch2/catch_test_macros.hpp>

#include <util/json.hpp>
#include <util/small_vector.hpp>

#include <engine/meta/serial.hpp>
#include <engine/meta/meta.hpp>

#include <engine/components/name_component.hpp>

#include <unordered_map>
#include <string>

TEST_CASE("engine::load", "[engine:meta]")
{
	engine::reflect_all();

	auto single_object_data = util::json::parse("{\"name\": \"Test Name\"}");

	SECTION("Load new object")
	{
		auto name = engine::load<engine::NameComponent>(single_object_data);

		REQUIRE(name.get_name() == "Test Name");
	}

	SECTION("Load with existing object, field assignment disabled")
	{
		auto name = engine::NameComponent { "Original Name" };

		engine::load(name, single_object_data, false);

		REQUIRE(name.get_name() == "Test Name");
	}

	SECTION("Load with existing object, field assignment enabled")
	{
		auto name = engine::NameComponent { "Original Name" };

		engine::load(name, single_object_data, true);

		REQUIRE(name.get_name() == "Test Name");
	}

	SECTION("Load vector of objects")
	{
		auto data = util::json::parse("[ \"A\", \"B\", \"C\" ]");

		auto vec = util::small_vector<engine::NameComponent, 3> {};

		engine::load(vec, data);

		REQUIRE(vec.size() == 3);

		REQUIRE(vec[0].get_name() == "A");
		REQUIRE(vec[1].get_name() == "B");
		REQUIRE(vec[2].get_name() == "C");
	}

	SECTION("Load object map")
	{
		auto data = util::json::parse("{ \"A\": \"A_Name\", \"B\": \"B_Name\" }");

		std::unordered_map<std::string, engine::NameComponent> name_map;

		engine::load(name_map, data);

		REQUIRE(name_map.size() == 2);

		REQUIRE(name_map["A"].get_name() == "A_Name");
		REQUIRE(name_map["B"].get_name() == "B_Name");
	}
}