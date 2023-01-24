#include <catch2/catch_test_macros.hpp>

#include <util/json.hpp>

#include <engine/meta/meta.hpp>
#include <engine/meta/meta_type_descriptor.hpp>
#include <engine/meta/serial.hpp>

#include <engine/reflection.hpp>

#include <engine/input/buttons.hpp>

#include <string_view>

namespace engine
{
	struct ReflectionTest
	{
		std::int32_t x = {};
		std::int32_t y = {};
		std::int32_t z = {};

		ReflectionTest(std::int32_t x, std::int32_t y, std::int32_t z)
			: x(x), y(y), z(z)
		{}

		ReflectionTest() {}
	};

	template <>
	void reflect<ReflectionTest>()
	{
		engine_meta_type<ReflectionTest>()
			.data<&ReflectionTest::x>("x"_hs)
			.data<&ReflectionTest::y>("y"_hs)
			.data<&ReflectionTest::z>("z"_hs)
			.ctor
			<
				decltype(ReflectionTest::x),
				decltype(ReflectionTest::y),
				decltype(ReflectionTest::z)
			>()
		;
	}
}

TEST_CASE("engine::meta_any_from_string", "[engine:meta]")
{
	engine::reflect_all();

	SECTION("Enum value")
	{
		auto result = engine::meta_any_from_string(std::string_view("Button::Jump"));

		REQUIRE(result);
		REQUIRE(result == engine::Button::Jump);
	}

	SECTION("Hash command executed on string representing enum value")
	{
		auto result = engine::meta_any_from_string(std::string_view("hash(\"Button::Jump\")"));

		REQUIRE(result);
		REQUIRE(result == engine::hash("Button::Jump").value());
	}

	SECTION("Hash shorthand used on string representing enum value")
	{
		auto result = engine::meta_any_from_string(std::string_view("#Button::Jump"));

		REQUIRE(result);
		REQUIRE(result == engine::hash("Button::Jump").value());
	}
}

TEST_CASE("engine::MetaTypeDescriptor", "[engine:meta]")
{
	using namespace entt::literals;
	using namespace nlohmann::literals;

	util::log::init();

	engine::reflect<engine::ReflectionTest>();

	SECTION("Direct value assignment")
	{
		auto descriptor = engine::MetaTypeDescriptor { "ReflectionTest"_hs };

		descriptor.set_variables_direct(10, 20, 30);

		auto instance = descriptor();

		REQUIRE(instance);
		REQUIRE(instance.try_cast<engine::ReflectionTest>());

		auto& raw_instance = *(instance.try_cast<engine::ReflectionTest>());

		REQUIRE(raw_instance.x == 10);
		REQUIRE(raw_instance.y == 20);
		REQUIRE(raw_instance.z == 30);
	}

	SECTION("CSV assignment")
	{
		auto descriptor = engine::MetaTypeDescriptor{ "ReflectionTest"_hs };

		descriptor.set_variables(std::string_view("1,2,3"));

		auto instance = descriptor();

		REQUIRE(instance);
		REQUIRE(instance.try_cast<engine::ReflectionTest>());

		auto& raw_instance = *(instance.try_cast<engine::ReflectionTest>());

		REQUIRE(raw_instance.x == 1);
		REQUIRE(raw_instance.y == 2);
		REQUIRE(raw_instance.z == 3);
	}

	SECTION("JSON assignment")
	{
		auto descriptor = engine::MetaTypeDescriptor{ "ReflectionTest"_hs };

		util::json content = "[123,456,789]"_json;

		descriptor.set_variables(content);

		auto instance = descriptor();

		REQUIRE(instance);
		REQUIRE(instance.try_cast<engine::ReflectionTest>());

		auto& raw_instance = *(instance.try_cast<engine::ReflectionTest>());

		REQUIRE(raw_instance.x == 123);
		REQUIRE(raw_instance.y == 456);
		REQUIRE(raw_instance.z == 789);
	}
}