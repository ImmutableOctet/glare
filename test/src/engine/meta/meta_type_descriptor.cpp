#include <catch2/catch_test_macros.hpp>

#include "reflection_test.hpp"

#include <engine/meta/meta_type_descriptor.hpp>
#include <engine/meta/hash.hpp>

#include <util/json.hpp>

TEST_CASE("engine::MetaTypeDescriptor", "[engine:meta]")
{
	using namespace engine::literals;
	using namespace util::literals;

	util::log::init();

	engine::reflect<engine::ReflectionTest>();

	SECTION("Direct value assignment")
	{
		auto descriptor = engine::MetaTypeDescriptor { "ReflectionTest"_hs };

		descriptor.set_variables_direct(10, 20, 30);

		auto instance = descriptor();

		REQUIRE(instance);
		
		auto* raw_instance = instance.try_cast<engine::ReflectionTest>();

		REQUIRE(raw_instance);

		REQUIRE(raw_instance->x == 10);
		REQUIRE(raw_instance->y == 20);
		REQUIRE(raw_instance->z == 30);
	}

	SECTION("CSV assignment")
	{
		auto descriptor = engine::MetaTypeDescriptor{ "ReflectionTest"_hs };

		descriptor.set_variables(std::string_view("1,2,3"));

		auto instance = descriptor();

		REQUIRE(instance);

		auto* raw_instance = instance.try_cast<engine::ReflectionTest>();

		REQUIRE(raw_instance);

		REQUIRE(raw_instance->x == 1);
		REQUIRE(raw_instance->y == 2);
		REQUIRE(raw_instance->z == 3);
	}

	SECTION("JSON assignment")
	{
		auto descriptor = engine::MetaTypeDescriptor{ "ReflectionTest"_hs };

		auto content = util::json::parse("[123,456,789]");

		descriptor.set_variables(content);

		auto instance = descriptor();

		REQUIRE(instance);
		
		auto* raw_instance = instance.try_cast<engine::ReflectionTest>();

		REQUIRE(raw_instance);

		REQUIRE(raw_instance->x == 123);
		REQUIRE(raw_instance->y == 456);
		REQUIRE(raw_instance->z == 789);
	}

	SECTION("Description from JSON")
	{
		const auto type = engine::resolve<engine::ReflectionTest>();

		auto type_desc = engine::MetaTypeDescriptor
		{
			type,

			util::json::parse("{ \"x\": 1, \"y\": 2, \"z\": 3, \"nested_value\": { \"value\": 4.0 } }")
		};

		REQUIRE(type_desc.size() == 4);

		REQUIRE(type_desc.field_names[0] == "x"_hs);

		const auto x = type_desc.field_values[0].try_cast<std::int32_t>();

		REQUIRE(x);
		REQUIRE((*x) == 1);

		REQUIRE(type_desc.field_names[1] == "y"_hs);

		const auto y = type_desc.field_values[1].try_cast<std::int32_t>();

		REQUIRE(y);
		REQUIRE((*y) == 2);

		REQUIRE(type_desc.field_names[2] == "z"_hs);

		const auto z = type_desc.field_values[2].try_cast<std::int32_t>();
		
		REQUIRE(z);
		REQUIRE((*z) == 3);

		REQUIRE(type_desc.field_names[3] == "nested_value"_hs);

		const auto nested_raw = type_desc.field_values[3].try_cast<engine::MetaTypeDescriptor>();

		REQUIRE(nested_raw);

		const auto& nested = *nested_raw;

		REQUIRE(nested.size() == 1);

		const auto nested_value = nested.field_values[0].try_cast<float>();

		REQUIRE(nested_value);
		REQUIRE((*nested_value) >= 4.0f);
	}
}