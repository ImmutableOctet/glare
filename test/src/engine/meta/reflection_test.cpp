#include <catch2/catch_test_macros.hpp>

#include "reflection_test.hpp"

#include <engine/reflection/reflection.hpp>
#include <engine/meta/hash.hpp>

namespace engine
{
	static std::int32_t free_function_as_member(const ReflectionTest& instance)
	{
		return (instance.z * 2);
	}

	template <>
	void reflect<ReflectionTest>()
	{
		engine_meta_type
		<
			ReflectionTest,

			MetaTypeReflectionConfig
			{
				.capture_standard_data_members = true,
				.generate_optional_reflection  = true,
				.generate_operator_wrappers    = true, // false;
				.generate_indirect_getters     = true,
				.generate_indirect_setters     = true,
				.generate_json_bindings        = true
			}
		>()
			.data<&ReflectionTest::x>("x"_hs)
			.data<&ReflectionTest::y>("y"_hs)
			.data<&ReflectionTest::z>("z"_hs)
			.data<&ReflectionTest::nested_value>("nested_value"_hs)
			
			.ctor
			<
				decltype(ReflectionTest::x),
				decltype(ReflectionTest::y),
				decltype(ReflectionTest::z)
			>()

			.ctor
			<
				decltype(ReflectionTest::x),
				decltype(ReflectionTest::y),
				decltype(ReflectionTest::z),
				decltype(ReflectionTest::nested_value)
			>()

			.func<&ReflectionTest::get_property_x>("get_property_x"_hs)
			.func<&ReflectionTest::set_property_x>("set_property_x"_hs)

			.func<&ReflectionTest::fn>("fn"_hs)
			.func<&ReflectionTest::const_method_test>("const_method_test"_hs)
			.func<&ReflectionTest::non_const_method_test>("non_const_method_test"_hs)
			.func<&free_function_as_member>("free_function_as_member"_hs)
			.func<&ReflectionTest::get_nested>("get_nested"_hs)
			.func<&ReflectionTest::const_opaque_method>("const_opaque_method"_hs)
			.func<&ReflectionTest::non_const_opaque_method>("non_const_opaque_method"_hs)
			.func<&ReflectionTest::method_with_context>("method_with_context"_hs)
			.func<&ReflectionTest::function_with_context>("function_with_context"_hs)
			.func<&ReflectionTest::get_optional_value>("get_optional_value"_hs)
			.func<static_cast<std::string (*)(std::int32_t value)>(&ReflectionTest::overloaded_function)>("overloaded_function"_hs)
			.func<static_cast<std::string (*)(const std::string& value)>(&ReflectionTest::overloaded_function)>("overloaded_function"_hs)
		;

		engine_meta_type<ReflectionTest::Nested>()
			.data<&ReflectionTest::Nested::value>("value"_hs)
			.func<&ReflectionTest::Nested::make_nested>("make_nested"_hs)
			.func<&ReflectionTest::Nested::get>("get"_hs)
			.ctor<decltype(ReflectionTest::Nested::value)>()
		;
	}
}

TEST_CASE("engine::reflect", "[engine:reflection]")
{
	using namespace engine::literals;

	engine::reflect<engine::ReflectionTest>();

	SECTION("Resolve type")
	{
		const auto type = engine::resolve<engine::ReflectionTest>();
		const auto type_id = type.id();

		const auto intended_type_id = ("ReflectionTest"_hs).value();
		const auto type_from_id = engine::resolve(intended_type_id);

		REQUIRE(type == type_from_id);
		REQUIRE(type_id == intended_type_id);
	}
}