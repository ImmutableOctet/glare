#include <catch2/catch_test_macros.hpp>

#include "reflection_test.hpp"

#include <util/json.hpp>
#include <util/small_vector.hpp>

#include <engine/meta/serial.hpp>
#include <engine/meta/meta.hpp>
#include <engine/meta/meta_type_descriptor.hpp>

#include <engine/components/name_component.hpp>

#include <unordered_map>
#include <string>
#include <string_view>
#include <cstdint>

// Debugging related:
#include <util/log.hpp>

TEST_CASE("engine::save", "[engine:meta]")
{
	SECTION("Standalone floating-point primitive")
	{
		auto as_json = engine::save(10.0f);

		REQUIRE(as_json.is_number_float());
		REQUIRE(as_json.get<float>() >= 10.0f);
	}

	SECTION("Standalone integer primitive")
	{
		auto as_json = engine::save(50);

		REQUIRE(as_json.is_number_integer());
		REQUIRE(as_json.get<std::int32_t>() == 50);
	}

	SECTION("Standalone string primitive")
	{
		auto as_json = engine::save(std::string { "Test String Value" });

		REQUIRE(as_json.is_string());
		REQUIRE(as_json.get<std::string>() == "Test String Value");
	}

	SECTION("Standalone string view primitive")
	{
		auto as_json = engine::save(std::string_view { "Test String Value" });

		REQUIRE(as_json.is_string());
		REQUIRE(as_json.get<std::string>() == "Test String Value");
	}

	SECTION("Simple object")
	{
		auto instance = engine::ReflectionTest { 1, 2, 3, engine::ReflectionTest::Nested { 4.0f } };

		auto as_json = engine::save(instance, true);

		const auto& x_as_json = as_json["x:int"];

		REQUIRE(!x_as_json.empty());
		REQUIRE(x_as_json.is_number_integer());

		const auto x_value = x_as_json.get<std::int32_t>();
		
		REQUIRE(x_value == 1);

		const auto& y_as_json = as_json["y:int"];

		REQUIRE(!y_as_json.empty());
		REQUIRE(y_as_json.is_number_integer());

		const auto y_value = y_as_json.get<std::int32_t>();

		REQUIRE(y_value == 2);

		const auto& z_as_json = as_json["z:int"];

		REQUIRE(!z_as_json.empty());
		REQUIRE(z_as_json.is_number_integer());

		const auto z_value = z_as_json.get<std::int32_t>();

		REQUIRE(z_value == 3);

		const auto& nested_as_json = as_json["nested_value:ReflectionTest::Nested"];

		REQUIRE(!nested_as_json.empty());
		REQUIRE(nested_as_json.is_object());

		const auto& nested_value_as_json = nested_as_json["value:float"];

		REQUIRE(!nested_value_as_json.empty());
		REQUIRE(nested_value_as_json.is_number_float());

		const auto nested_value = nested_value_as_json.get<float>();

		REQUIRE(nested_value >= 4.0f);
	}

	SECTION("MetaTypeDescriptor")
	{
		const auto type = engine::resolve<engine::ReflectionTest>();

		const auto source_json = util::json::parse("{ \"x\": 1, \"y\": 2, \"z\": 3, \"nested_value\": { \"value\": 4.0 } }");

		REQUIRE(source_json.size() == 4);

		auto type_desc = engine::MetaTypeDescriptor
		{
			type,

			source_json
		};

		REQUIRE(type_desc.size() == source_json.size());

		const auto result_json = engine::save(type_desc, false);

		REQUIRE(result_json.size() == type_desc.size());

		REQUIRE(result_json["x"] == source_json["x"]);
		REQUIRE(result_json["y"] == source_json["y"]);
		REQUIRE(result_json["z"] == source_json["z"]);

		//REQUIRE(result_json["nested_value"] == source_json["nested_value"]);

		REQUIRE(result_json["nested_value"].size() == source_json["nested_value"].size());
		REQUIRE(result_json["nested_value"]["value"].get<float>() >= source_json["nested_value"]["value"].get<float>());
	}
}

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