#include <catch2/catch_test_macros.hpp>

#include "reflection_test.hpp"

#include <util/json.hpp>
#include <util/small_vector.hpp>

#include <util/binary/memory_stream.hpp>

#include <engine/meta/serial.hpp>
#include <engine/meta/meta.hpp>
#include <engine/meta/meta_type_descriptor.hpp>

#include <engine/components/name_component.hpp>

#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <limits>
#include <cstdint>

// Debugging related:
#include <util/log.hpp>

TEST_CASE("engine::save_binary, engine::load_binary", "[engine:meta]")
{
	using FormatSpec = engine::BinaryFormatConfig::Format;
	using Version = engine::BinaryFormatConfig::FormatVersion;
	using StringFormat = engine::StringBinaryFormat;

	auto instance = engine::ReflectionTest { 10, 20, 30, engine::ReflectionTest::Nested { 40.0f } };

	auto binary_stream = util::MemoryStream { 1024, false };

	SECTION("Save and load minimal binary in memory")
	{
		const auto binary_format = engine::BinaryFormatConfig
		{
			Version {},
			FormatSpec::None, // static_cast<FormatSpec>(FormatSpec::None),
			StringFormat::Default
		};

		REQUIRE(engine::save_binary(instance, binary_stream, binary_format));

		auto copy_of_instance = engine::load_binary<decltype(instance)>(binary_stream, binary_format);

		REQUIRE(copy_of_instance.x == instance.x);
		REQUIRE(copy_of_instance.y == instance.y);
		REQUIRE(copy_of_instance.z == instance.z);
		REQUIRE(copy_of_instance.nested_value.value >= instance.nested_value.value);
	}

	SECTION("Save and load detailed binary in memory")
	{
		const auto binary_format = engine::BinaryFormatConfig
		{
			// NOTE: Dummy version number.
			Version { 255 },

			// NOTE: `All` is not recommended for normal use.
			FormatSpec::All,

			StringFormat::Default
		};

		REQUIRE(engine::save_binary(instance, binary_stream, binary_format));

		// NOTE: When using `FormatSpec::All`, `StandardHeader` is enabled, meaning the
		// binary format is embedded in the data already, allowing us to forgo specifying it here.
		auto copy_of_instance = engine::load_binary<decltype(instance)>(binary_stream);

		REQUIRE(copy_of_instance.x == instance.x);
		REQUIRE(copy_of_instance.y == instance.y);
		REQUIRE(copy_of_instance.z == instance.z);
		REQUIRE(copy_of_instance.nested_value.value >= instance.nested_value.value);
	}

	SECTION("Save and load integral primitives")
	{
		constexpr auto i8  = std::numeric_limits<std::int8_t>::max();
		constexpr auto i16 = std::numeric_limits<std::int16_t>::max();
		constexpr auto i32 = std::numeric_limits<std::int32_t>::max();
		constexpr auto i64 = std::numeric_limits<std::int64_t>::max();

		constexpr auto u8  = std::numeric_limits<std::uint8_t>::max();
		constexpr auto u16 = std::numeric_limits<std::uint16_t>::max();
		constexpr auto u32 = std::numeric_limits<std::uint32_t>::max();
		constexpr auto u64 = std::numeric_limits<std::uint64_t>::max();

		REQUIRE(engine::save_binary(i8, binary_stream));
		REQUIRE(engine::save_binary(i16, binary_stream));
		REQUIRE(engine::save_binary(i32, binary_stream));
		REQUIRE(engine::save_binary(i64, binary_stream));
		
		REQUIRE(engine::save_binary(u8, binary_stream));
		REQUIRE(engine::save_binary(u16, binary_stream));
		REQUIRE(engine::save_binary(u32, binary_stream));
		REQUIRE(engine::save_binary(u64, binary_stream));

		REQUIRE(engine::load_binary<decltype(i8)>(binary_stream) == i8);
		REQUIRE(engine::load_binary<decltype(i16)>(binary_stream) == i16);
		REQUIRE(engine::load_binary<decltype(i32)>(binary_stream) == i32);
		REQUIRE(engine::load_binary<decltype(i64)>(binary_stream) == i64);

		REQUIRE(engine::load_binary<decltype(u8)>(binary_stream) == u8);
		REQUIRE(engine::load_binary<decltype(u16)>(binary_stream) == u16);
		REQUIRE(engine::load_binary<decltype(u32)>(binary_stream) == u32);
		REQUIRE(engine::load_binary<decltype(u64)>(binary_stream) == u64);
	}

	SECTION("Save and load floating-point primitives")
	{
		const auto f32 = static_cast<float>(0.25);
		const auto f64 = static_cast<double>(0.5);
		//const auto f80 = static_cast<long double>(1.0);

		REQUIRE(engine::save_binary(f32, binary_stream));
		REQUIRE(engine::save_binary(f64, binary_stream));
		//REQUIRE(engine::save_binary(f80, binary_stream));

		REQUIRE(engine::load_binary<decltype(f32)>(binary_stream) >= f32);
		REQUIRE(engine::load_binary<decltype(f64)>(binary_stream) >= f64);
		//REQUIRE(engine::load_binary<decltype(f80)>(binary_stream) >= f80);
	}

	SECTION("Save and load string primitive")
	{
		const auto string_content = std::string_view { "Binary format string" };

		engine::save_binary(std::string { string_content }, binary_stream);

		const auto string_from_binary = engine::load_binary<std::string>(binary_stream);

		REQUIRE(string_from_binary == string_content);
	}
}

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

	SECTION("Load standard vector of objects")
	{
		auto data = util::json::parse("[ \"A\", \"B\", \"C\" ]");

		auto vec = std::vector<engine::NameComponent> {};

		engine::load(vec, data);

		REQUIRE(vec.size() == 3);

		REQUIRE(vec[0].get_name() == "A");
		REQUIRE(vec[1].get_name() == "B");
		REQUIRE(vec[2].get_name() == "C");
	}

	SECTION("Load small vector of objects")
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