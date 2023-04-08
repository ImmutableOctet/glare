#include <catch2/catch_test_macros.hpp>

#include <engine/meta/meta.hpp>
#include <engine/meta/types.hpp>
#include <engine/meta/variant_wrapper.hpp>

#include <cstdint>
#include <string>

TEST_CASE("engine::VariantWrapper", "[engine:meta]")
{
	engine::reflect_all();

	SECTION("Basic usage")
	{
		using wrapper_t = engine::VariantWrapper<std::int32_t, float, std::string>;

		wrapper_t v;

		v = std::string { "Test" };

		REQUIRE(v.type_index() == 2);
		REQUIRE(static_cast<bool>(v));
		REQUIRE(v.get<std::string>() == "Test");
	}

	SECTION("Empty")
	{
		engine::VariantWrapper<std::monostate, std::int32_t> v;

		REQUIRE(!v);
	}

	SECTION("From MetaAny with conversion, no monostate")
	{
		using wrapper_t = engine::VariantWrapper<std::int32_t, float>;

		engine::MetaAny value = std::string { "10" };

		auto w = wrapper_t::from_meta_any(value);

		REQUIRE(w);
		REQUIRE(w->type_index() == 0);
		REQUIRE(w->get<int>() == 10);
	}

	SECTION("From MetaAny without conversion, no monostate")
	{
		using wrapper_t = engine::VariantWrapper<std::int32_t, float, std::string>;

		engine::MetaAny value = std::string { "100" };

		auto w = wrapper_t::from_meta_any(value);

		REQUIRE(w);
		REQUIRE(w->type_index() == 2);
		REQUIRE(w->get<std::string>() == "100");
	}

	SECTION("From MetaAny with monostate, no conversion")
	{
		using wrapper_t = engine::VariantWrapper<std::monostate, std::int32_t, float>;

		engine::MetaAny value = std::string { "1000" };

		auto w = wrapper_t::from_meta_any(value, false);

		REQUIRE(!w);
		REQUIRE(w.type_index() == 0);
	}

	SECTION("From MetaAny with conversion, with monostate")
	{
		using wrapper_t = engine::VariantWrapper<std::monostate, std::int32_t, float>;

		engine::MetaAny value = std::string { "10000" };

		auto w = wrapper_t::from_meta_any(value);

		REQUIRE(w);
		REQUIRE(w.type_index() == 1);
		REQUIRE(w.get<std::int32_t>() == 10000);
	}

	SECTION("Comparison")
	{
		using wrapper_t = engine::VariantWrapper<std::monostate, std::int32_t, float>;

		auto first  = wrapper_t { 10 };
		auto second = wrapper_t { 10 };
		auto third  = wrapper_t { 10.0f };
		auto fourth = wrapper_t { 10.0f };
		auto fifth  = wrapper_t {};
		auto sixth  = wrapper_t {};

		REQUIRE(first == second);
		REQUIRE(first != third);
		REQUIRE(second != third);
		REQUIRE(third == fourth);
		REQUIRE(first != fifth);
		REQUIRE(third != fifth);
		REQUIRE(fifth == sixth);
	}
}