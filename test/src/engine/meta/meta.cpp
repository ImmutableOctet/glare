#include <catch2/catch_test_macros.hpp>

#include <util/json.hpp>

#include <engine/meta/meta.hpp>
#include <engine/meta/meta_type_descriptor.hpp>
#include <engine/meta/serial.hpp>
#include <engine/meta/meta_value_operation.hpp>
#include <engine/meta/meta_value_operator.hpp>
#include <engine/meta/meta_variable_context.hpp>
#include <engine/meta/meta_variable_evaluation_context.hpp>
#include <engine/meta/meta_evaluation_context.hpp>
#include <engine/meta/meta_variable_scope.hpp>
#include <engine/meta/indirect_meta_any.hpp>
#include <engine/meta/indirect_meta_variable_target.hpp>

#include <engine/entity/entity_shared_storage.hpp>
#include <engine/entity/entity_descriptor.hpp>
#include <engine/entity/entity_variables.hpp>
#include <engine/entity/entity_shared_storage.hpp>
#include <engine/entity/components/instance_component.hpp>

#include <engine/components/name_component.hpp>

#include <engine/resource_manager/entity_factory_data.hpp>

#include <engine/types.hpp>
#include <engine/reflection.hpp>

#include <engine/input/buttons.hpp>

#include <string_view>
#include <string>
#include <memory>
#include <variant>
#include <unordered_map>

// Debugging related:
#include <util/parse.hpp>

namespace engine
{
	struct ReflectionTest
	{
		struct Nested
		{
			static Nested make_nested(float value)
			{
				return { value };
			}

			float get() const { return value; }

			Nested(float value={}) : value(value) {}

			float value;

			bool operator==(const Nested&) const noexcept = default;
			bool operator!=(const Nested&) const noexcept = default;
		};

		std::int32_t x = {};
		std::int32_t y = {};
		std::int32_t z = {};

		Nested nested_value = {};

		ReflectionTest(std::int32_t x, std::int32_t y, std::int32_t z, const Nested& nested_value={})
			: x(x), y(y), z(z), nested_value(nested_value)
		{}

		ReflectionTest() {}

		/*
		ReflectionTest(const ReflectionTest& r)
			: ReflectionTest(r.x, r.y, r.z, r.nested_value)
		{
			print("Copy created.");
		}
		*/

		ReflectionTest(const ReflectionTest&) = default;

		ReflectionTest(ReflectionTest&&) noexcept = default;

		ReflectionTest& operator=(const ReflectionTest&) = default;
		ReflectionTest& operator=(ReflectionTest&&) noexcept = default;

		bool operator==(const ReflectionTest&) const noexcept = default;
		bool operator!=(const ReflectionTest&) const noexcept = default;

		inline bool operator>(const ReflectionTest& instance) const // noexcept
		{
			return (static_cast<std::int32_t>(*this) > static_cast<std::int32_t>(instance));
		}

		inline bool operator<(const ReflectionTest& instance) const // noexcept
		{
			return (static_cast<std::int32_t>(*this) < static_cast<std::int32_t>(instance));
		}

		inline operator std::int32_t() const noexcept { return (x + y + z); }

		static ReflectionTest fn(std::int32_t value=1)
		{
			return { value*1, value*2, value*3 };
		}

		inline ReflectionTest operator+(const ReflectionTest& instance) const
		{
			return { (x + instance.x), (y + instance.y), (z + instance.z), { nested_value.value + instance.nested_value.value } };
		}

		std::int32_t const_method_test() const { return y; }
		std::int32_t non_const_method_test() { return z; }

		std::int32_t const_opaque_method() const
		{
			const auto sum_value = static_cast<std::int32_t>(*this);

			return (sum_value * sum_value);
		}

		std::int32_t non_const_opaque_method()
		{
			x++;
			y++;
			z++;

			return static_cast<std::int32_t>(*this);
		}

		static std::int32_t function_with_context(Registry& registry, Entity entity)
		{
			return static_cast<std::int32_t>(entity);
		}

		Entity method_with_context(Registry& registry, Entity entity)
		{
			return entity;
		}

		Nested& get_nested() { return nested_value; }
	};

	std::int32_t free_function_as_member(const ReflectionTest& instance)
	{
		return (instance.z * 2);
	}

	template <>
	void reflect<ReflectionTest>()
	{
		engine_meta_type<ReflectionTest>(true, true, true) // false
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

			.func<&ReflectionTest::fn>("fn"_hs)
			.func<&ReflectionTest::const_method_test>("const_method_test"_hs)
			.func<&ReflectionTest::non_const_method_test>("non_const_method_test"_hs)
			.func<&free_function_as_member>("free_function_as_member"_hs)
			.func<&ReflectionTest::get_nested>("get_nested"_hs)
			.func<&ReflectionTest::const_opaque_method>("const_opaque_method"_hs)
			.func<&ReflectionTest::non_const_opaque_method>("non_const_opaque_method"_hs)
			.func<&ReflectionTest::method_with_context>("method_with_context"_hs)
			.func<&ReflectionTest::function_with_context>("function_with_context"_hs)
		;

		engine_meta_type<ReflectionTest::Nested>()
			.data<&ReflectionTest::Nested::value>("value"_hs)
			.func<&ReflectionTest::Nested::make_nested>("make_nested"_hs)
			.func<&ReflectionTest::Nested::get>("get"_hs)
			.ctor<decltype(ReflectionTest::Nested::value)>()
		;
	}
}

TEST_CASE("engine::MetaValueOperator", "[engine:meta]")
{
	SECTION("parse_value_operator")
	{
		REQUIRE(std::get<0>(*engine::parse_value_operator("+", true)) == engine::MetaValueOperator::UnaryPlus);
		REQUIRE(std::get<0>(*engine::parse_value_operator("-", true)) == engine::MetaValueOperator::UnaryMinus);
		
		REQUIRE(std::get<0>(*engine::parse_value_operator("~", true)) == engine::MetaValueOperator::BitwiseNot);
		REQUIRE(!engine::parse_value_operator("~", false));

		REQUIRE(std::get<0>(*engine::parse_value_operator("!", true)) == engine::MetaValueOperator::LogicalNot);
		REQUIRE(!engine::parse_value_operator("!", false));

		REQUIRE(std::get<0>(*engine::parse_value_operator("*", true)) == engine::MetaValueOperator::Dereference);

		REQUIRE(std::get<0>(*engine::parse_value_operator("*", false)) == engine::MetaValueOperator::Multiply);
		REQUIRE(std::get<0>(*engine::parse_value_operator("/", false)) == engine::MetaValueOperator::Divide);
		REQUIRE(std::get<0>(*engine::parse_value_operator("%", false)) == engine::MetaValueOperator::Modulus);

		REQUIRE(std::get<0>(*engine::parse_value_operator("+", false)) == engine::MetaValueOperator::Add);
		REQUIRE(std::get<0>(*engine::parse_value_operator("-", false)) == engine::MetaValueOperator::Subtract);

		REQUIRE(std::get<0>(*engine::parse_value_operator("<<", false)) == engine::MetaValueOperator::ShiftLeft);
		REQUIRE(std::get<0>(*engine::parse_value_operator(">>", false)) == engine::MetaValueOperator::ShiftRight);

		REQUIRE(std::get<0>(*engine::parse_value_operator("<", false)) == engine::MetaValueOperator::LessThan);
		REQUIRE(std::get<0>(*engine::parse_value_operator("<=", false)) == engine::MetaValueOperator::LessThanOrEqual);
		REQUIRE(std::get<0>(*engine::parse_value_operator(">", false)) == engine::MetaValueOperator::GreaterThan);
		REQUIRE(std::get<0>(*engine::parse_value_operator(">=", false)) == engine::MetaValueOperator::GreaterThanOrEqual);

		REQUIRE(std::get<0>(*engine::parse_value_operator("==", false)) == engine::MetaValueOperator::Equal);
		REQUIRE(std::get<0>(*engine::parse_value_operator("!=", false)) == engine::MetaValueOperator::NotEqual);
		REQUIRE(std::get<0>(*engine::parse_value_operator("<>", false)) == engine::MetaValueOperator::NotEqual);

		REQUIRE(std::get<0>(*engine::parse_value_operator("&", false)) == engine::MetaValueOperator::BitwiseAnd);
		REQUIRE(std::get<0>(*engine::parse_value_operator("^", false)) == engine::MetaValueOperator::BitwiseXOR);
		REQUIRE(std::get<0>(*engine::parse_value_operator("|", false)) == engine::MetaValueOperator::BitwiseOr);

		REQUIRE(std::get<0>(*engine::parse_value_operator("&&", false)) == engine::MetaValueOperator::LogicalAnd);
		REQUIRE(std::get<0>(*engine::parse_value_operator("||", false)) == engine::MetaValueOperator::LogicalOr);

		REQUIRE(std::get<0>(*engine::parse_value_operator("=", false)) == engine::MetaValueOperator::Assign);

		REQUIRE(std::get<0>(*engine::parse_value_operator("+=", false)) == engine::MetaValueOperator::AddAssign);
		REQUIRE(std::get<0>(*engine::parse_value_operator("-=", false)) == engine::MetaValueOperator::SubtractAssign);
		REQUIRE(std::get<0>(*engine::parse_value_operator("*=", false)) == engine::MetaValueOperator::MultiplyAssign);
		REQUIRE(std::get<0>(*engine::parse_value_operator("/=", false)) == engine::MetaValueOperator::DivideAssign);
		REQUIRE(std::get<0>(*engine::parse_value_operator("%=", false)) == engine::MetaValueOperator::ModulusAssign);
		REQUIRE(std::get<0>(*engine::parse_value_operator("<<=", false)) == engine::MetaValueOperator::ShiftLeftAssign);
		REQUIRE(std::get<0>(*engine::parse_value_operator(">>=", false)) == engine::MetaValueOperator::ShiftRightAssign);
		REQUIRE(std::get<0>(*engine::parse_value_operator("&=", false)) == engine::MetaValueOperator::BitwiseAndAssign);
		REQUIRE(std::get<0>(*engine::parse_value_operator("^=", false)) == engine::MetaValueOperator::BitwiseXORAssign);
		REQUIRE(std::get<0>(*engine::parse_value_operator("|=", false)) == engine::MetaValueOperator::BitwiseOrAssign);

		REQUIRE(std::get<0>(*engine::parse_value_operator("::", false, true)) == engine::MetaValueOperator::Get);
		REQUIRE(std::get<0>(*engine::parse_value_operator(".", false, true)) == engine::MetaValueOperator::Get);
		REQUIRE(std::get<0>(*engine::parse_value_operator("->", false, true)) == engine::MetaValueOperator::Get);

		REQUIRE(std::get<0>(*engine::parse_value_operator("[", false, true)) == engine::MetaValueOperator::Subscript);
		REQUIRE(std::get<0>(*engine::parse_value_operator("]", false, true)) == engine::MetaValueOperator::Subscript);
		REQUIRE(std::get<0>(*engine::parse_value_operator("[]", false, true)) == engine::MetaValueOperator::Subscript);
	}

	SECTION("decay_operation")
	{

		REQUIRE(engine::decay_operation(engine::MetaValueOperator::AddAssign) == engine::MetaValueOperator::Add);
		REQUIRE(engine::decay_operation(engine::MetaValueOperator::SubtractAssign) == engine::MetaValueOperator::Subtract);
		REQUIRE(engine::decay_operation(engine::MetaValueOperator::MultiplyAssign) == engine::MetaValueOperator::Multiply);
		REQUIRE(engine::decay_operation(engine::MetaValueOperator::DivideAssign) == engine::MetaValueOperator::Divide);
		REQUIRE(engine::decay_operation(engine::MetaValueOperator::ModulusAssign) == engine::MetaValueOperator::Modulus);
		REQUIRE(engine::decay_operation(engine::MetaValueOperator::ShiftLeftAssign) == engine::MetaValueOperator::ShiftLeft);
		REQUIRE(engine::decay_operation(engine::MetaValueOperator::ShiftRightAssign) == engine::MetaValueOperator::ShiftRight);
		REQUIRE(engine::decay_operation(engine::MetaValueOperator::BitwiseAndAssign) == engine::MetaValueOperator::BitwiseAnd);
		REQUIRE(engine::decay_operation(engine::MetaValueOperator::BitwiseXORAssign) == engine::MetaValueOperator::BitwiseXOR);
		REQUIRE(engine::decay_operation(engine::MetaValueOperator::BitwiseOrAssign) == engine::MetaValueOperator::BitwiseOr);

		REQUIRE(engine::decay_operation(engine::MetaValueOperator::Add) == engine::MetaValueOperator::Add);
		REQUIRE(engine::decay_operation(engine::MetaValueOperator::Assign) == engine::MetaValueOperator::Assign);
	}

	SECTION("decay_operator_name")
	{
		using namespace engine::literals;

		REQUIRE(engine::decay_operator_name(engine::MetaValueOperator::AddAssign) == "operator+"_hs);
		REQUIRE(engine::decay_operator_name(engine::MetaValueOperator::SubtractAssign) == "operator-"_hs);
		REQUIRE(engine::decay_operator_name(engine::MetaValueOperator::MultiplyAssign) == "operator*"_hs);
		REQUIRE(engine::decay_operator_name(engine::MetaValueOperator::DivideAssign) == "operator/"_hs);
		REQUIRE(engine::decay_operator_name(engine::MetaValueOperator::ModulusAssign) == "operator%"_hs);
		REQUIRE(engine::decay_operator_name(engine::MetaValueOperator::ShiftLeftAssign) == "operator<<"_hs);
		REQUIRE(engine::decay_operator_name(engine::MetaValueOperator::ShiftRightAssign) == "operator>>"_hs);
		REQUIRE(engine::decay_operator_name(engine::MetaValueOperator::BitwiseAndAssign) == "operator&"_hs);
		REQUIRE(engine::decay_operator_name(engine::MetaValueOperator::BitwiseXORAssign) == "operator^"_hs);
		REQUIRE(engine::decay_operator_name(engine::MetaValueOperator::BitwiseOrAssign) == "operator|"_hs);

		REQUIRE(engine::decay_operator_name(engine::MetaValueOperator::Add) == "operator+"_hs);
		REQUIRE(engine::decay_operator_name(engine::MetaValueOperator::Assign) == "operator="_hs);
	}
}

TEST_CASE("engine::resolve_meta_any", "[engine:meta]")
{
	engine::reflect_all();

	SECTION("String Map population from object")
	{
		using native_map_type = std::unordered_map<std::string, std::string>;

		auto map_instance = native_map_type{};
		auto map_type = engine::resolve<native_map_type>();

		REQUIRE(map_type);

		auto store = engine::resolve_meta_any(util::json::parse("{ \"A\": \"B\", \"C\": \"D\" }"), map_type);

		REQUIRE(store);
		
		auto result = engine::try_get_underlying_value(store, map_instance);

		REQUIRE(result);

		auto* as_map = result.try_cast<native_map_type>();

		REQUIRE(as_map);
		REQUIRE(as_map == &map_instance);
		REQUIRE(as_map->size() == 2);

		REQUIRE(map_instance["A"] == "B");
		REQUIRE(map_instance["C"] == "D");
	}

	SECTION("String Map construction from object")
	{
		using native_map_type = std::unordered_map<std::string, std::string>;

		auto vector_type_factory = entt::meta<native_map_type>();

		auto map_type = engine::resolve<native_map_type>();

		REQUIRE(map_type);

		auto store = engine::resolve_meta_any(util::json::parse("{ \"First\": \"First Value\", \"Second\": \"Second Value\" }"), map_type);

		REQUIRE(store);
		
		auto result = engine::try_get_underlying_value(store);

		REQUIRE(result);

		auto* as_map = result.try_cast<native_map_type>();

		REQUIRE(as_map);
		REQUIRE(as_map->size() == 2);

		REQUIRE((*as_map)["First"] == "First Value");
		REQUIRE((*as_map)["Second"] == "Second Value");
	}

	SECTION("Vector population from array")
	{
		using native_vector_type = util::small_vector<std::int32_t, 4>;

		auto vector_instance = native_vector_type {};
		auto vector_type = engine::resolve<native_vector_type>();

		REQUIRE(vector_type);

		auto store = engine::resolve_meta_any(util::json::parse("[10, 11, 12]"), vector_type);

		REQUIRE(store);
		
		auto result = engine::try_get_underlying_value(store, vector_instance);

		REQUIRE(result);

		auto* as_vector = result.try_cast<native_vector_type>();

		REQUIRE(as_vector);
		REQUIRE(as_vector == &vector_instance);
		REQUIRE(as_vector->size() == 3);

		REQUIRE(vector_instance[0] == 10);
		REQUIRE(vector_instance[1] == 11);
		REQUIRE(vector_instance[2] == 12);
	}

	SECTION("Vector construction from array")
	{
		using native_vector_type = util::small_vector<std::int32_t, 4>;

		auto vector_type_factory = entt::meta<native_vector_type>();

		auto vector_type = engine::resolve<native_vector_type>();

		REQUIRE(vector_type);

		auto store = engine::resolve_meta_any(util::json::parse("[7, 5, 8, 3]"), vector_type);

		REQUIRE(store);

		auto result = engine::try_get_underlying_value(store);

		REQUIRE(result);

		auto* as_vector = result.try_cast<native_vector_type>();

		REQUIRE(as_vector);
		REQUIRE(as_vector->size() == 4);

		REQUIRE((*as_vector)[0] == 7);
		REQUIRE((*as_vector)[1] == 5);
		REQUIRE((*as_vector)[2] == 8);
		REQUIRE((*as_vector)[3] == 3);
	}
}

TEST_CASE("engine::meta_any_from_string", "[engine:meta]")
{
	engine::reflect_all();
	engine::reflect<engine::ReflectionTest>();

	SECTION("Subscript operator")
	{
		util::small_vector<std::int32_t, 3> vec;

		vec.push_back(10);
		vec.push_back(20);
		vec.push_back(30);

		auto subscript_expr = engine::meta_any_from_string
		(
			std::string_view("[2]"),
			{
				.allow_subscript_semantics = true
			}
		);

		REQUIRE(subscript_expr);

		auto subscript_result = engine::try_get_underlying_value(subscript_expr, vec);

		REQUIRE(subscript_result);

		auto as_integer = subscript_result.try_cast<std::int32_t>();

		REQUIRE(as_integer);
		REQUIRE((*as_integer) == vec[2]);
	}

	SECTION("Opaque vector push_back")
	{
		util::small_vector<std::int32_t, 3> vec;

		vec.push_back(1);
		vec.push_back(10);

		auto push_back_expr = engine::meta_any_from_string
		(
			std::string_view("push_back(100)"),
			{
				.allow_function_call_semantics = true,
				.allow_opaque_function_references = true
			}
		);

		REQUIRE(push_back_expr);

		auto push_back_result = engine::try_get_underlying_value(push_back_expr, vec);

		REQUIRE(push_back_result);
		REQUIRE(vec.size() == 3);
		REQUIRE(vec[2] == 100);

		auto as_iterator = push_back_result.try_cast<entt::meta_sequence_container::iterator>();

		REQUIRE(as_iterator);
		REQUIRE((*(*as_iterator)) == vec[2]);
	}

	SECTION("Opaque vector `at` function")
	{
		util::small_vector<std::int32_t, 5> vec;

		vec.push_back(2);
		vec.push_back(3);
		vec.push_back(5);
		vec.push_back(7);
		vec.push_back(11);

		auto at_expr = engine::meta_any_from_string
		(
			std::string_view("at(2)"),
			{
				.allow_function_call_semantics = true,
				.allow_opaque_function_references = true
			}
		);

		REQUIRE(at_expr);

		auto at_result = engine::try_get_underlying_value(at_expr, vec);

		REQUIRE(at_result);

		auto as_iterator = at_result.try_cast<entt::meta_sequence_container::iterator>();

		REQUIRE(as_iterator);
		REQUIRE((*(*as_iterator)) == vec[2]);
	}

	SECTION("Opaque vector insert")
	{
		util::small_vector<std::int32_t, 4> vec;

		vec.push_back(10);
		vec.push_back(1000);
		vec.push_back(10000);

		auto insert_expr = engine::meta_any_from_string
		(
			std::string_view("insert(at(1), 100)"),
			{
				.allow_function_call_semantics = true,
				.allow_opaque_function_references = true
			}
		);

		REQUIRE(insert_expr);

		auto insert_result = engine::try_get_underlying_value(insert_expr, vec);

		REQUIRE(insert_result);
		REQUIRE(vec.size() == 4);
		REQUIRE(vec[1] == 100);

		auto as_iterator = insert_result.try_cast<entt::meta_sequence_container::iterator>();

		REQUIRE(as_iterator);
		REQUIRE((*(*as_iterator)) == vec[1]);
	}

	SECTION("Method call with forwarded context")
	{
		engine::Registry registry;
		auto entity = registry.create();

		auto instance = engine::ReflectionTest { 5, 10, 15 };

		auto context_fn_expr = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest::method_with_context"),
			{
				.allow_function_call_semantics = true
			}
		);

		REQUIRE(context_fn_expr);

		auto context_fn_expr_result = engine::try_get_underlying_value(context_fn_expr, instance, registry, entity);

		REQUIRE(context_fn_expr_result);

		auto context_fn_expr_result_as_entity = context_fn_expr_result.try_cast<engine::Entity>();

		REQUIRE(context_fn_expr_result_as_entity);
		REQUIRE((*context_fn_expr_result_as_entity) == entity);
	}

	SECTION("Function call with forwarded context")
	{
		engine::Registry registry;
		auto entity = registry.create();

		auto instance = engine::ReflectionTest(5, 10, 15);

		auto context_fn_expr = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest::function_with_context"),
			{
				.allow_function_call_semantics = true
			}
		);

		REQUIRE(context_fn_expr);

		auto context_fn_expr_result = engine::try_get_underlying_value(context_fn_expr, registry, entity);

		REQUIRE(context_fn_expr_result);

		auto context_fn_expr_result_as_entity_int = context_fn_expr_result.try_cast<std::int32_t>();

		REQUIRE(context_fn_expr_result_as_entity_int);
		REQUIRE((*context_fn_expr_result_as_entity_int) == static_cast<std::int32_t>(entity));
	}

	SECTION("Opaque member reference")
	{
		engine::MetaVariableContext variable_declaration_context;

		variable_declaration_context.define_variable(engine::MetaVariableScope::Local, "test_var");

		engine::EntityVariables<1> variables;

		auto runtime_variable_context = engine::MetaVariableEvaluationContext { &variables };

		auto evaluation_context = engine::MetaEvaluationContext
		{
			&runtime_variable_context
		};

		REQUIRE(runtime_variable_context.declare(variable_declaration_context) == 1);

		// NOTE: Placeholder registry and entity added due to missing `MetaEvaluationContext`-only overloads internally.
		// TODO: Fix this limitation.
		engine::Registry placeholder_registry;
		engine::Entity   placeholder_entity = placeholder_registry.create(); // null;

		auto variable_assignment = engine::meta_any_from_string
		(
			std::string_view("test_var = ReflectionTest(500, 1000, 2000)"),
			{
				.context = { &variable_declaration_context },

				.allow_explicit_type_construction = true,
				.allow_variable_assignment = true
			}
		);

		REQUIRE(variable_assignment);

		auto variable_assignment_result = engine::try_get_underlying_value
		(
			variable_assignment,
			placeholder_registry, placeholder_entity,
			evaluation_context
		);

		REQUIRE(variable_assignment_result);

		auto variable_opaque_member_expr = engine::meta_any_from_string
		(
			std::string_view("test_var.y"),
			{
				.context = { &variable_declaration_context },

				.allow_member_references = true,
				.allow_opaque_member_references = true
			}
		);

		REQUIRE(variable_opaque_member_expr);

		auto variable_opaque_member_expr_result = engine::try_get_underlying_value
		(
			variable_opaque_member_expr,
			placeholder_registry, placeholder_entity,
			evaluation_context
		);

		REQUIRE(variable_opaque_member_expr_result);

		auto variable_opaque_member_expr_result_as_int = variable_opaque_member_expr_result.try_cast<std::int32_t>();

		REQUIRE(variable_opaque_member_expr_result_as_int);
		REQUIRE((*variable_opaque_member_expr_result_as_int) == 1000);
	}

	SECTION("Opaque member-function calls")
	{
		engine::MetaVariableContext variable_declaration_context;

		variable_declaration_context.define_variable(engine::MetaVariableScope::Local, "first");
		variable_declaration_context.define_variable(engine::MetaVariableScope::Local, "second");

		engine::EntityVariables<2> variables;

		auto runtime_variable_context = engine::MetaVariableEvaluationContext { &variables };

		auto evaluation_context = engine::MetaEvaluationContext
		{
			&runtime_variable_context
		};

		REQUIRE(runtime_variable_context.declare(variable_declaration_context) == 2);

		// NOTE: Placeholder registry and entity added due to missing `MetaEvaluationContext`-only overloads internally.
		// TODO: Fix this limitation.
		engine::Registry placeholder_registry;
		engine::Entity   placeholder_entity = placeholder_registry.create(); // null;

		auto first_variable_assignment = engine::meta_any_from_string
		(
			std::string_view("first = ReflectionTest(1, 2, 3)"),
			{
				.context = { &variable_declaration_context },

				.allow_explicit_type_construction = true,
				.allow_variable_assignment = true
			}
		);

		REQUIRE(first_variable_assignment);

		auto first_variable_assignment_result = engine::try_get_underlying_value
		(
			first_variable_assignment,
			placeholder_registry, placeholder_entity,
			evaluation_context
		);

		REQUIRE(first_variable_assignment_result);

		auto non_const_opaque_method_call = engine::meta_any_from_string
		(
			std::string_view("first.non_const_opaque_method()"),
			{
				.context = { &variable_declaration_context },

				.allow_opaque_function_references = true
			}
		);

		REQUIRE(non_const_opaque_method_call);

		auto non_const_opaque_method_call_result = engine::try_get_underlying_value
		(
			non_const_opaque_method_call,
			placeholder_registry, placeholder_entity,
			evaluation_context
		);

		REQUIRE(non_const_opaque_method_call_result);

		auto non_const_opaque_method_call_result_as_int = non_const_opaque_method_call_result.try_cast<std::int32_t>();

		REQUIRE(non_const_opaque_method_call_result_as_int);
		REQUIRE((*non_const_opaque_method_call_result_as_int) == (1+1 + 2+1 + 3+1));

		auto first_var_definition = variable_declaration_context.retrieve_variable("first");

		REQUIRE(first_var_definition);

		auto first_var = runtime_variable_context.get(first_var_definition->scope, first_var_definition->resolved_name);

		REQUIRE(first_var);

		auto first_var_raw = first_var.try_cast<engine::ReflectionTest>();

		REQUIRE(first_var_raw);
		REQUIRE(first_var_raw->x == (1+1));
		REQUIRE(first_var_raw->y == (2+1));
		REQUIRE(first_var_raw->z == (3+1));

		auto second_variable_assignment = engine::meta_any_from_string
		(
			std::string_view("second = ReflectionTest(1, 22, 333)"),
			{
				.context = { &variable_declaration_context },

				.allow_explicit_type_construction = true,
				.allow_variable_assignment = true
			}
		);

		REQUIRE(second_variable_assignment);

		auto second_variable_assignment_result = engine::try_get_underlying_value
		(
			second_variable_assignment,
			placeholder_registry, placeholder_entity,
			evaluation_context
		);

		REQUIRE(second_variable_assignment_result);

		auto const_opaque_method_call = engine::meta_any_from_string
		(
			std::string_view("second.const_opaque_method()"),
			{
				.context = { &variable_declaration_context },

				.allow_opaque_function_references = true
			}
		);

		REQUIRE(const_opaque_method_call);

		auto const_opaque_method_call_result = engine::try_get_underlying_value
		(
			const_opaque_method_call,
			placeholder_registry, placeholder_entity,
			evaluation_context
		);

		REQUIRE(const_opaque_method_call_result);

		auto const_opaque_method_call_result_as_int = const_opaque_method_call_result.try_cast<std::int32_t>();

		const auto expected_sum = ((1) + (22) + (333));

		REQUIRE(const_opaque_method_call_result_as_int);
		REQUIRE((*const_opaque_method_call_result_as_int) == (expected_sum * expected_sum));

		auto second_var_definition = variable_declaration_context.retrieve_variable("second");

		REQUIRE(second_var_definition);

		auto second_var = runtime_variable_context.get(second_var_definition->scope, second_var_definition->resolved_name);

		REQUIRE(second_var);

		auto second_var_raw = second_var.try_cast<engine::ReflectionTest>();

		REQUIRE(second_var_raw);
		REQUIRE(second_var_raw->x == (1));
		REQUIRE(second_var_raw->y == (22));
		REQUIRE(second_var_raw->z == (333));
	}

	SECTION("Unquoted plain text as string")
	{
		auto unquoted_string_literal = engine::meta_any_from_string(std::string_view("Hello world"));

		REQUIRE(unquoted_string_literal);

		auto as_string = unquoted_string_literal.try_cast<std::string>();

		REQUIRE(as_string);
		REQUIRE((*as_string) == "Hello world");
	}

	// TODO: Implement a test section dedicated to invoking an `IndirectMetaVariableTarget` generated from `meta_any_from_string`.
	SECTION("Indirect variable reference to other thread's local (Self entity target)")
	{
		using namespace engine::literals;

		auto fully_qualified_variable_reference = engine::meta_any_from_string
		(
			std::string_view("self.some_thread.some_var"),
			{
				.allow_entity_indirection = true,
				.allow_remote_variable_references = true
			}
		);

		REQUIRE(fully_qualified_variable_reference);

		auto* as_indirect_variable_target = fully_qualified_variable_reference.try_cast<engine::IndirectMetaVariableTarget>();

		REQUIRE(as_indirect_variable_target);

		REQUIRE(as_indirect_variable_target->target.is_self_targeted());
		REQUIRE(std::get<engine::EntityThreadID>(as_indirect_variable_target->thread.value) == "some_thread"_hs);
		REQUIRE(as_indirect_variable_target->variable.scope == engine::MetaVariableScope::Local);
		REQUIRE(as_indirect_variable_target->variable.name == engine::MetaVariableContext::resolve_path("some_thread"_hs, "some_var"_hs, engine::MetaVariableScope::Local));
	}

	SECTION("Indirect variable reference to other thread's local (No entity target)")
	{
		using namespace engine::literals;

		auto partially_qualified_variable_reference = engine::meta_any_from_string
		(
			std::string_view("thread_name.var_name"),
			{
				.allow_entity_indirection         = true,
				.allow_remote_variable_references = true
			}
		);

		REQUIRE(partially_qualified_variable_reference);

		auto* as_indirect_variable_target = partially_qualified_variable_reference.try_cast<engine::IndirectMetaVariableTarget>();

		REQUIRE(as_indirect_variable_target);

		REQUIRE(as_indirect_variable_target->target.is_self_targeted());
		REQUIRE(std::get<engine::EntityThreadID>(as_indirect_variable_target->thread.value) == "thread_name"_hs);
		REQUIRE(as_indirect_variable_target->variable.scope == engine::MetaVariableScope::Local);
		REQUIRE(as_indirect_variable_target->variable.name == engine::MetaVariableContext::resolve_path("thread_name"_hs, "var_name"_hs, engine::MetaVariableScope::Local));
	}

	SECTION("Indirect variable reference to other entity's thread-local variable")
	{
		using namespace engine::literals;

		auto fully_qualified_variable_reference = engine::meta_any_from_string
		(
			std::string_view("child(some_child_name).child_thread_name.local_variable_name"),
			{
				.allow_entity_indirection         = true,
				.allow_remote_variable_references = true
			}
		);

		REQUIRE(fully_qualified_variable_reference);

		auto* as_indirect_variable_target = fully_qualified_variable_reference.try_cast<engine::IndirectMetaVariableTarget>();

		REQUIRE(as_indirect_variable_target);

		REQUIRE(as_indirect_variable_target->target.is_child_target());
		REQUIRE(std::get<engine::EntityThreadID>(as_indirect_variable_target->thread.value) == "child_thread_name"_hs);
		REQUIRE(as_indirect_variable_target->variable.scope == engine::MetaVariableScope::Local);
		REQUIRE(as_indirect_variable_target->variable.name == engine::MetaVariableContext::resolve_path("child_thread_name"_hs, "local_variable_name"_hs, engine::MetaVariableScope::Local));
	}

	SECTION("Indirect variable reference to other entity's global variable")
	{
		using namespace engine::literals;

		auto entity_qualified_variable_reference = engine::meta_any_from_string
		(
			std::string_view("child(some_child_name).global_variable_name"),
			{
				.allow_entity_indirection         = true,
				.allow_remote_variable_references = true
			}
		);

		REQUIRE(entity_qualified_variable_reference);

		auto* as_indirect_variable_target = entity_qualified_variable_reference.try_cast<engine::IndirectMetaVariableTarget>();

		REQUIRE(as_indirect_variable_target);

		REQUIRE(as_indirect_variable_target->target.is_child_target());
		REQUIRE(as_indirect_variable_target->thread.empty());
		REQUIRE(as_indirect_variable_target->variable.scope == engine::MetaVariableScope::Global);
		REQUIRE(as_indirect_variable_target->variable.name == engine::MetaVariableContext::resolve_path({}, "global_variable_name"_hs, engine::MetaVariableScope::Global));
	}

	SECTION("Entity target")
	{
		using namespace engine::literals;

		auto entity_target_expr = engine::meta_any_from_string
		(
			std::string_view("entity(test_entity_name)"),
			{
				.allow_entity_indirection         = true,
				.allow_remote_variable_references = true
			}
		);

		REQUIRE(entity_target_expr);

		auto as_entity_target = entity_target_expr.try_cast<engine::EntityTarget>();

		REQUIRE(as_entity_target);
		REQUIRE(as_entity_target->is_entity_name_target());
		REQUIRE(std::get<engine::EntityTarget::EntityNameTarget>(as_entity_target->type).entity_name == "test_entity_name"_hs);
	}

	SECTION("Deferred entity target")
	{
		using namespace engine::literals;

		engine::Registry registry;

		auto target_entity = registry.create();

		registry.emplace<engine::NameComponent>(target_entity, "test_entity_name");

		auto entity_storage = engine::EntitySharedStorage {};

		auto entity_target_expr = engine::meta_any_from_string
		(
			std::string_view("entity(\"test_entity\" + \"_name\")"),
			{
				.storage = &entity_storage,

				.allow_entity_indirection         = true,
				.allow_remote_variable_references = true
			}
		);

		REQUIRE(entity_target_expr);

		auto entity_target_expr_interface = entity_target_expr.try_cast<engine::EntityTarget>();

		REQUIRE(entity_target_expr_interface);

		REQUIRE(entity_target_expr_interface->is_indirect_target());

		const auto& inner_expr_ref = std::get<engine::EntityTarget::IndirectTarget>(entity_target_expr_interface->type).target_value_expr;

		auto inner_expr = inner_expr_ref.get(entity_storage);

		REQUIRE(inner_expr);

		// NOTE: We don't check against the result of `entity_target_expr`/`entity_target_expr_interface`
		// here due to the requirement of an `InstanceComponent` attachment.
		// (i.e. doing this would bloat this test into something larger than it needs to be)
	}

	SECTION("Boolean literals")
	{
		auto true_expr = engine::meta_any_from_string
		(
			std::string_view("true"),
			{
				.resolve_symbol         = true,
				.allow_boolean_literals = true
			}
		);

		REQUIRE(true_expr);

		auto true_expr_as_bool = true_expr.try_cast<bool>();

		REQUIRE(true_expr_as_bool);
		REQUIRE((*true_expr_as_bool) == true);

		auto false_expr = engine::meta_any_from_string
		(
			std::string_view("false"),
			{
				.resolve_symbol         = true,
				.allow_boolean_literals = true
			}
		);

		REQUIRE(false_expr);

		auto false_expr_as_bool = false_expr.try_cast<bool>();

		REQUIRE(false_expr_as_bool);
		REQUIRE((*false_expr_as_bool) == false);
	}

	SECTION("Basic boolean cast")
	{
		auto expr = engine::meta_any_from_string
		(
			std::string_view("(1+1):bool")
		);

		REQUIRE(expr);

		auto result = engine::try_get_underlying_value(expr);

		REQUIRE(result);

		auto as_bool = result.try_cast<bool>();

		REQUIRE(as_bool);
		REQUIRE(*as_bool);
	}

	SECTION("Basic equality operator")
	{
		auto expr = engine::meta_any_from_string
		(
			std::string_view("10 == 10")
		);

		REQUIRE(expr);

		auto result = engine::try_get_underlying_value(expr);

		REQUIRE(result);

		auto as_bool = result.try_cast<bool>();

		REQUIRE(as_bool);
		REQUIRE(*as_bool);
	}

	SECTION("Basic inequality operator")
	{
		auto expr = engine::meta_any_from_string
		(
			std::string_view("5 != 0")
		);

		REQUIRE(expr);

		auto result = engine::try_get_underlying_value(expr);

		REQUIRE(result);

		auto as_bool = result.try_cast<bool>();

		REQUIRE(as_bool);
		REQUIRE(*as_bool);
	}

	SECTION("Alternative basic inequality operator")
	{
		auto expr = engine::meta_any_from_string
		(
			std::string_view("100 <> 50")
		);

		REQUIRE(expr);

		auto result = engine::try_get_underlying_value(expr);

		REQUIRE(result);

		auto as_bool = result.try_cast<bool>();

		REQUIRE(as_bool);
		REQUIRE(*as_bool);
	}

	SECTION("Basic greater than operator")
	{
		auto expr = engine::meta_any_from_string
		(
			std::string_view("40 > 30")
		);

		REQUIRE(expr);

		auto result = engine::try_get_underlying_value(expr);

		REQUIRE(result);

		auto as_bool = result.try_cast<bool>();

		REQUIRE(as_bool);
		REQUIRE(*as_bool);
	}

	SECTION("Basic less than operator")
	{
		auto expr = engine::meta_any_from_string
		(
			std::string_view("20 < 30")
		);

		REQUIRE(expr);

		auto result = engine::try_get_underlying_value(expr);

		REQUIRE(result);

		auto as_bool = result.try_cast<bool>();

		REQUIRE(as_bool);
		REQUIRE(*as_bool);
	}

	SECTION("Basic greater than or equal to operator")
	{
		auto greater_expr = engine::meta_any_from_string
		(
			std::string_view("31 >= 30")
		);

		REQUIRE(greater_expr);

		auto greater_result = engine::try_get_underlying_value(greater_expr);

		REQUIRE(greater_result);

		auto greater_result_as_bool = greater_result.try_cast<bool>();

		REQUIRE(greater_result_as_bool);
		REQUIRE(*greater_result_as_bool);

		auto equal_expr = engine::meta_any_from_string
		(
			std::string_view("30 >= 30")
		);

		REQUIRE(equal_expr);

		auto equal_result = engine::try_get_underlying_value(equal_expr);

		REQUIRE(equal_result);

		auto equal_result_as_bool = equal_result.try_cast<bool>();

		REQUIRE(equal_result_as_bool);
		REQUIRE(*equal_result_as_bool);
	}

	SECTION("Basic less than or equal to operator")
	{
		auto lesser_expr = engine::meta_any_from_string
		(
			std::string_view("39 <= 40")
		);

		REQUIRE(lesser_expr);

		auto lesser_result = engine::try_get_underlying_value(lesser_expr);

		REQUIRE(lesser_result);

		auto lesser_result_as_bool = lesser_result.try_cast<bool>();

		REQUIRE(lesser_result_as_bool);
		REQUIRE(*lesser_result_as_bool);

		auto equal_expr = engine::meta_any_from_string
		(
			std::string_view("40 <= 40")
		);

		REQUIRE(equal_expr);

		auto equal_result = engine::try_get_underlying_value(equal_expr);

		REQUIRE(equal_result);

		auto equal_result_as_bool = equal_result.try_cast<bool>();

		REQUIRE(equal_result_as_bool);
		REQUIRE(*equal_result_as_bool);
	}

	SECTION("Advanced greater than operator usage (Custom type)")
	{
		auto equality_expr = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest(2, 2, 3) > ReflectionTest(1, 2, 3)"),
			{ .allow_explicit_type_construction = true }
		);

		REQUIRE(equality_expr);

		auto result = engine::try_get_underlying_value(equality_expr);

		REQUIRE(result);

		auto as_bool = result.try_cast<bool>();

		REQUIRE(as_bool);
		REQUIRE(*as_bool);
	}

	SECTION("Advanced greater than or equal operator usage (Custom type)")
	{
		auto equality_expr = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest(1, 2, 3) >= ReflectionTest(1, 2, 3)"),
			{ .allow_explicit_type_construction = true }
		);

		REQUIRE(equality_expr);

		auto result = engine::try_get_underlying_value(equality_expr);

		REQUIRE(result);

		auto as_bool = result.try_cast<bool>();

		REQUIRE(as_bool);
		REQUIRE(*as_bool);
	}

	SECTION("Advanced less than operator usage (Custom type)")
	{
		auto equality_expr = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest(1, 1, 3) < ReflectionTest(1, 2, 3)"),
			{ .allow_explicit_type_construction = true }
		);

		REQUIRE(equality_expr);

		auto result = engine::try_get_underlying_value(equality_expr);

		REQUIRE(result);

		auto as_bool = result.try_cast<bool>();

		REQUIRE(as_bool);
		REQUIRE(*as_bool);
	}

	SECTION("Advanced less than or equal operator usage (Custom type)")
	{
		auto equality_expr = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest(1, 2, 2) <= ReflectionTest(1, 2, 3)"),
			{ .allow_explicit_type_construction = true }
		);

		REQUIRE(equality_expr);

		auto result = engine::try_get_underlying_value(equality_expr);

		REQUIRE(result);

		auto as_bool = result.try_cast<bool>();

		REQUIRE(as_bool);
		REQUIRE(*as_bool);
	}

	SECTION("Advanced equality operator usage (Custom type)")
	{
		auto equality_expr = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest(1, 2, 3) == ReflectionTest(1, 2, 3)"),
			{ .allow_explicit_type_construction = true }
		);

		REQUIRE(equality_expr);

		auto result = engine::try_get_underlying_value(equality_expr);

		REQUIRE(result);

		auto as_bool = result.try_cast<bool>();

		REQUIRE(as_bool);
		REQUIRE(*as_bool);
	}

	SECTION("Advanced inequality operator usage (Custom type)")
	{
		auto equality_expr = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest(10, 20, 30) != ReflectionTest(10, 20, 3)"),
			{ .allow_explicit_type_construction = true }
		);

		REQUIRE(equality_expr);

		auto result = engine::try_get_underlying_value(equality_expr);

		REQUIRE(result);

		auto as_bool = result.try_cast<bool>();

		REQUIRE(as_bool);
		REQUIRE(*as_bool);
	}

	SECTION("Advanced equality operator usage (Strings)")
	{
		auto equality_expr = engine::meta_any_from_string
		(
			std::string_view("\"(Test==)\" == \"(Test==)\"")
		);

		REQUIRE(equality_expr);

		auto result = engine::try_get_underlying_value(equality_expr);

		REQUIRE(result);

		auto as_bool = result.try_cast<bool>();

		REQUIRE(as_bool);
		REQUIRE(*as_bool);
	}

	SECTION("Advanced inequality operator usage (Strings)")
	{
		auto equality_expr = engine::meta_any_from_string
		(
			std::string_view("\"(Test==)\" != \"(+Test)\"")
		);

		REQUIRE(equality_expr);

		auto result = engine::try_get_underlying_value(equality_expr);

		REQUIRE(result);

		auto as_bool = result.try_cast<bool>();

		REQUIRE(as_bool);
		REQUIRE(*as_bool);
	}

	SECTION("Cast operator")
	{
		auto cast_expr = engine::meta_any_from_string
		(
			std::string_view("5:float"),
			{ .allow_explicit_type_construction = true }
		);

		REQUIRE(cast_expr);

		auto as_float = cast_expr.try_cast<float>();

		REQUIRE(as_float);
		REQUIRE(*as_float >= 5.0f);
	}

	SECTION("Simplistic scoped expression + Cast operator")
	{
		auto cast_expr = engine::meta_any_from_string
		(
			std::string_view("(5 + 4):float"),
			{ .allow_explicit_type_construction = true }
		);

		REQUIRE(cast_expr);

		auto result = engine::get_indirect_value_or_ref(cast_expr);

		REQUIRE(result);

		auto as_float = result.try_cast<float>();

		REQUIRE(as_float);
		REQUIRE(*as_float >= 9.0f);
	}

	SECTION("Immediate cast operator")
	{
		auto cast_expr = engine::meta_any_from_string
		(
			std::string_view("125:float - 14:float"),
			{ .allow_explicit_type_construction = true }
		);

		REQUIRE(cast_expr);

		auto result = engine::get_indirect_value_or_ref(cast_expr);

		REQUIRE(result);

		auto as_float = result.try_cast<float>();

		REQUIRE(as_float);
		REQUIRE(*as_float >= 111.0f);
	}

	SECTION("Advanced scoped expression + Cast operator")
	{
		auto cast_expr = engine::meta_any_from_string
		(
			std::string_view("((5:float + 10:float + 3.25): int) : string"),
			{ .allow_explicit_type_construction = true }
		);

		REQUIRE(cast_expr);

		auto result = engine::get_indirect_value_or_ref(cast_expr);

		REQUIRE(result);

		auto as_float = result.try_cast<float>();
		auto as_int = result.try_cast<std::int32_t>();

		auto as_string = result.try_cast<std::string>();

		REQUIRE(as_string);
		REQUIRE(*as_string == "18");
	}

	SECTION("Variable unary operators")
	{
		engine::MetaVariableContext variable_declaration_context;

		variable_declaration_context.define_variable(engine::MetaVariableScope::Local, "test_var");

		engine::EntityVariables<1> variables;

		auto runtime_variable_context = engine::MetaVariableEvaluationContext { &variables };

		auto evaluation_context = engine::MetaEvaluationContext
		{
			&runtime_variable_context
		};

		REQUIRE(runtime_variable_context.declare(variable_declaration_context) == 1);

		// NOTE: Placeholder registry and entity added due to missing `MetaEvaluationContext`-only overloads internally.
		// TODO: Fix this limitation.
		engine::Registry placeholder_registry;
		engine::Entity   placeholder_entity = placeholder_registry.create(); // null;

		auto initial_assignment = engine::meta_any_from_string
		(
			std::string_view("test_var:bool = false"),
			{
				.context = { &variable_declaration_context },
				.allow_variable_assignment = true
			}
		);

		REQUIRE(initial_assignment);

		auto initial_assignment_result = engine::try_get_underlying_value(initial_assignment, placeholder_registry, placeholder_entity, evaluation_context);

		REQUIRE(initial_assignment_result);

		auto initial_assignment_result_underlying = engine::try_get_underlying_value(initial_assignment_result, placeholder_registry, placeholder_entity, evaluation_context);

		REQUIRE(initial_assignment_result_underlying);

		auto initial_assignment_result_raw = initial_assignment_result_underlying.try_cast<bool>();

		REQUIRE(initial_assignment_result_raw);
		REQUIRE(!(*initial_assignment_result_raw));

		auto unary_not_operator = engine::meta_any_from_string
		(
			std::string_view("!test_var"),
			{
				.context = { &variable_declaration_context }
			}
		);

		auto unary_not_operator_result = engine::try_get_underlying_value(unary_not_operator, placeholder_registry, placeholder_entity, evaluation_context);

		REQUIRE(unary_not_operator_result);

		auto unary_not_operator_result_raw = unary_not_operator_result.try_cast<bool>();

		REQUIRE(unary_not_operator_result_raw);
		REQUIRE(*unary_not_operator_result_raw);

		auto var_boolean_inversion = engine::meta_any_from_string
		(
			std::string_view("test_var = !test_var"),
			{
				.context = { &variable_declaration_context },
				.allow_variable_assignment = true
			}
		);

		REQUIRE(var_boolean_inversion);

		auto var_boolean_inversion_result = engine::try_get_underlying_value(var_boolean_inversion, placeholder_registry, placeholder_entity, evaluation_context);

		REQUIRE(var_boolean_inversion_result);

		auto test_var_definition = variable_declaration_context.retrieve_variable("test_var");

		REQUIRE(test_var_definition);

		auto test_var = runtime_variable_context.get(test_var_definition->scope, test_var_definition->resolved_name);

		REQUIRE(test_var);

		auto test_var_raw = test_var.try_cast<bool>();

		REQUIRE(test_var_raw);
		REQUIRE((*test_var_raw));
	}

	SECTION("Variable assignment")
	{
		engine::MetaVariableContext variable_declaration_context;

		variable_declaration_context.define_variable(engine::MetaVariableScope::Local, "a");
		variable_declaration_context.define_variable(engine::MetaVariableScope::Local, "b");

		engine::EntityVariables<2> variables;

		auto runtime_variable_context = engine::MetaVariableEvaluationContext { &variables };

		auto evaluation_context = engine::MetaEvaluationContext
		{
			&runtime_variable_context
		};

		REQUIRE(runtime_variable_context.declare(variable_declaration_context) == 2);

		// NOTE: Placeholder registry and entity added due to missing `MetaEvaluationContext`-only overloads internally.
		// TODO: Fix this limitation.
		engine::Registry placeholder_registry;
		engine::Entity   placeholder_entity = placeholder_registry.create(); // null;

		auto initial_assignment = engine::meta_any_from_string
		(
			std::string_view("b:int = 101.05"),
			{
				.context = { &variable_declaration_context },
				.allow_variable_assignment = true
			}
		);

		REQUIRE(initial_assignment);

		auto initial_assignment_result = engine::try_get_underlying_value(initial_assignment, placeholder_registry, placeholder_entity, evaluation_context);

		REQUIRE(initial_assignment_result);

		auto initial_assignment_result_underlying = engine::try_get_underlying_value(initial_assignment_result, placeholder_registry, placeholder_entity, evaluation_context);

		REQUIRE(initial_assignment_result_underlying);

		auto initial_assignment_result_raw = initial_assignment_result_underlying.try_cast<std::int32_t>();

		REQUIRE(initial_assignment_result_raw);
		REQUIRE(*initial_assignment_result_raw == 101);

		auto var_b_definition = variable_declaration_context.retrieve_variable("b");

		REQUIRE(var_b_definition);

		auto var_b = runtime_variable_context.get(var_b_definition->scope, var_b_definition->resolved_name);

		REQUIRE(var_b);

		auto var_b_raw = var_b.try_cast<std::int32_t>();

		REQUIRE(var_b_raw);
		REQUIRE(*var_b_raw == 101);

		auto variable_to_variable_assignment = engine::meta_any_from_string
		(
			std::string_view("a :float= b - 1"),
			{
				.context = { &variable_declaration_context },
				.allow_variable_assignment = true
			}
		);

		REQUIRE(variable_to_variable_assignment);

		auto variable_to_variable_assignment_result = engine::try_get_underlying_value(variable_to_variable_assignment, placeholder_registry, placeholder_entity, evaluation_context);

		REQUIRE(variable_to_variable_assignment_result);

		auto var_a_definition = variable_declaration_context.retrieve_variable("a");

		REQUIRE(var_a_definition);

		auto var_a = runtime_variable_context.get(var_a_definition->scope, var_a_definition->resolved_name);

		REQUIRE(var_a);

		auto var_a_raw = var_a.try_cast<float>();

		REQUIRE(var_a_raw);
		REQUIRE(*var_a_raw >= 100.0f);

		auto variable_to_variable_assignment_result_underlying = engine::try_get_underlying_value(variable_to_variable_assignment_result, placeholder_registry, placeholder_entity, evaluation_context);

		REQUIRE(variable_to_variable_assignment_result_underlying);

		auto variable_to_variable_assignment_result_raw = variable_to_variable_assignment_result_underlying.try_cast<float>(); // std::int32_t

		REQUIRE(variable_to_variable_assignment_result_raw);
		REQUIRE(*variable_to_variable_assignment_result_raw >= static_cast<float>((*initial_assignment_result_raw) - 1)); // == *initial_assignment_result_raw

		auto instance_member_assignment = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest::x :int = a * b:float"),
			{
				.context = { &variable_declaration_context },
				.allow_member_references = true,
				.allow_variable_assignment = true
			}
		);

		REQUIRE(instance_member_assignment);

		auto instance = engine::ReflectionTest(12, 23, 34);

		auto instance_member_assignment_result = engine::try_get_underlying_value(instance_member_assignment, instance, placeholder_registry, placeholder_entity, evaluation_context);

		REQUIRE(instance_member_assignment_result);

		auto instance_member_assignment_result_underlying = engine::try_get_underlying_value(instance_member_assignment_result, instance, placeholder_registry, placeholder_entity, evaluation_context);

		REQUIRE(instance_member_assignment_result_underlying);

		auto instance_member_assignment_result_raw = instance_member_assignment_result_underlying.try_cast<std::int32_t>();

		REQUIRE(instance_member_assignment_result_raw);
		REQUIRE(*instance_member_assignment_result_raw == 10100);
		
		REQUIRE(instance.x == *instance_member_assignment_result_raw);

		auto instance_member_concatenate = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest::x :int += a / 5.0 + 2:float + float(7)"),
			{
				.context = { &variable_declaration_context },
				.allow_member_references = true,
				.allow_explicit_type_construction = true,
				.allow_variable_assignment = true
			}
		);

		auto instance_member_concatenate_result = engine::try_get_underlying_value(instance_member_concatenate, instance, placeholder_registry, placeholder_entity, evaluation_context);

		REQUIRE(instance_member_concatenate_result);

		auto instance_member_concatenate_result_underlying = engine::try_get_underlying_value(instance_member_concatenate_result, instance, placeholder_registry, placeholder_entity, evaluation_context);

		REQUIRE(instance_member_concatenate_result_underlying);

		auto instance_member_concatenate_result_raw = instance_member_concatenate_result_underlying.try_cast<std::int32_t>();

		REQUIRE(*instance_member_concatenate_result_raw == ((*instance_member_assignment_result_raw) + static_cast<std::int32_t>(*var_a_raw / 5.0f) + static_cast<std::int32_t>(2.0f) + static_cast<std::int32_t>(static_cast<float>(7))));
		REQUIRE(instance.x == *instance_member_concatenate_result_raw);
	}

	SECTION("Explicit type construction")
	{
		auto value = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest(1, 2, 3, 4.0)"),
			{ .allow_explicit_type_construction = true }
		);

		REQUIRE(value);

		auto as_instance = value.try_cast<engine::ReflectionTest>();

		REQUIRE(as_instance);

		REQUIRE(as_instance->x == 1);
		REQUIRE(as_instance->y == 2);
		REQUIRE(as_instance->z == 3);
		REQUIRE(as_instance->nested_value.value >= 4.0);
	}

	SECTION("Explicit type construction + operator use")
	{
		auto value = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest(1, 2, 3) + ReflectionTest(4, 5, 6) + ReflectionTest(7, 8, 9, 1.0)"),
			{
				.allow_explicit_type_construction = true,
				.resolve_value_operations = true
			}
		);

		auto result = engine::try_get_underlying_value(value);

		REQUIRE(result);

		auto as_instance = result.try_cast<engine::ReflectionTest>();

		REQUIRE(as_instance);

		REQUIRE(as_instance->x == 12);
		REQUIRE(as_instance->y == 15);
		REQUIRE(as_instance->z == 18);
		REQUIRE(as_instance->nested_value.value >= 1.0);
	}

	SECTION("Implicit type construction")
	{
		auto value = engine::meta_any_from_string
		(
			std::string_view("Name"),
			{ .allow_implicit_type_construction = true },
			engine::resolve<engine::NameComponent>(),
			false
		);

		REQUIRE(value);

		auto* as_name_component = value.try_cast<engine::NameComponent>();

		REQUIRE(as_name_component);

		REQUIRE(as_name_component->get_name() == "Name");
	}
	

	SECTION("Command result + member access")
	{
		auto value = engine::meta_any_from_string
		(
			std::string_view("thread(some_thread_name).empty"),
			{
				.allow_member_references = true,
				.allow_value_resolution_commands = true
			}
		);

		REQUIRE(value);

		auto as_thread_target_empty = value.try_cast<bool>();

		REQUIRE(as_thread_target_empty);

		const auto result = *as_thread_target_empty;

		REQUIRE(result == false);
	}
	
	// TODO: SECTION("Command result + function call") {}

	SECTION("Const method call")
	{
		auto const_getter = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest::const_method_test"),
			{ .allow_function_call_semantics = true }
		);

		REQUIRE(const_getter);

		auto instance = engine::ReflectionTest { 0, 123, 0 };

		auto result = engine::try_get_underlying_value(const_getter, instance);

		REQUIRE(result);

		auto as_integer = result.try_cast<std::int32_t>();

		REQUIRE(as_integer);

		REQUIRE((*as_integer) == 123);
	}

	SECTION("Non-const method call")
	{
		auto non_const_getter = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest::non_const_method_test()"),
			{ .allow_function_call_semantics = true }
		);

		REQUIRE(non_const_getter);

		auto instance = engine::ReflectionTest{ 0, 0, 13 };

		auto result = engine::try_get_underlying_value(non_const_getter, instance);

		REQUIRE(result);

		auto as_integer = result.try_cast<std::int32_t>();

		REQUIRE(as_integer);

		REQUIRE((*as_integer) == 13);
	}

	SECTION("Free function as member-function call")
	{
		auto getter = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest::free_function_as_member()"),
			{ .allow_function_call_semantics = true }
		);

		REQUIRE(getter);

		auto instance = engine::ReflectionTest { 0, 0, 13 };

		auto result = engine::try_get_underlying_value(getter, instance);

		REQUIRE(result);

		auto as_integer = result.try_cast<std::int32_t>();

		REQUIRE(as_integer);

		REQUIRE((*as_integer) == 26);
	}

	SECTION("Method call + member access")
	{
		auto getter = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest::get_nested()::value"),
			{
				.allow_member_references = true,
				.allow_function_call_semantics = true
			}
		);

		REQUIRE(getter);

		auto instance = engine::ReflectionTest { 0, 0, 0, { 42.0f } };

		auto result = engine::try_get_underlying_value(getter, instance);

		REQUIRE(result);

		auto as_float = result.try_cast<float>();

		REQUIRE(as_float);

		REQUIRE((*as_float) >= 42.0f);
	}

	SECTION("Function call + member access")
	{
		auto nested_value_from_function_call = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest::Nested::make_nested(100.0)::value"),
			{
				.allow_member_references = true,
				.allow_function_call_semantics = true
			}
		);

		REQUIRE(nested_value_from_function_call);

		auto result = engine::try_get_underlying_value(nested_value_from_function_call);

		REQUIRE(result);

		auto as_float = result.try_cast<float>();

		REQUIRE(as_float);

		REQUIRE((*as_float) >= 100.0f);
	}

	SECTION("Method call + function call")
	{
		auto nested_value = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest::Nested::make_nested(27.0)::get()"),
			{
				.allow_member_references = true,
				.allow_function_call_semantics = true
			}
		);

		REQUIRE(nested_value);

		auto result = engine::try_get_underlying_value(nested_value);

		REQUIRE(result);

		auto as_float = result.try_cast<float>();

		REQUIRE(as_float);

		REQUIRE((*as_float) >= 27.0f);
	}
	
	SECTION("Function call + member reference + math")
	{
		auto value = engine::meta_any_from_string
		(
			std::string_view("2.0 + ReflectionTest::Nested::make_nested(10.0 + 2.0)::value + 2.0"),
			{
				.allow_member_references = true,
				.allow_function_call_semantics = true,

				.resolve_value_operations = true
			}
		);

		REQUIRE(value);

		auto result = engine::try_get_underlying_value(value);

		REQUIRE(result);

		auto as_float = result.try_cast<float>();

		REQUIRE(as_float);

		REQUIRE((*as_float) >= 16.0);
	}

	SECTION("Entity component member reference")
	{
		auto value = engine::meta_any_from_string
		(
			std::string_view("self.NameComponent::name"),
			{
				.allow_member_references = true,
				.allow_entity_indirection = true
			}
		);

		REQUIRE(value);

		engine::Registry registry;
		
		auto entity = registry.create();

		registry.emplace<engine::NameComponent>(entity, "Entity Name");

		auto name_resolved = engine::get_indirect_value_or_ref(value, registry, entity);

		REQUIRE(name_resolved);

		auto as_string = name_resolved.try_cast<std::string>();

		REQUIRE(as_string);

		REQUIRE(*as_string == "Entity Name");
	}

	SECTION("Entity component member reference + shared storage")
	{
		auto storage = std::make_shared<const engine::EntityFactoryData>();

		engine::Registry registry;
		
		auto entity = registry.create();
		auto& instance = registry.emplace<engine::InstanceComponent>(entity, storage);
		registry.emplace<engine::ReflectionTest>(entity, 1, 2, 3);

		auto value = engine::meta_any_from_string
		(
			std::string_view("self.ReflectionTest::z"),
			{
				.storage = &(instance.get_storage()),

				.allow_member_references = true,
				.allow_entity_indirection = true
			}
		);

		REQUIRE(value);

		auto resolved = engine::get_indirect_value_or_ref(value, registry, entity);

		REQUIRE(resolved);

		auto as_int = resolved.try_cast<std::int32_t>();

		REQUIRE(as_int);

		REQUIRE(*as_int == 3);
	}

	SECTION("Entity target + component member reference + string concatenation operators")
	{
		auto value = engine::meta_any_from_string
		(
			std::string_view("test_ + self.NameComponent::name + \"_a\" + _b"), // 0.2 * 
			{
				.allow_member_references = true,
				.allow_entity_indirection = true
			}
		);

		REQUIRE(value);

		engine::Registry registry;
		
		auto entity = registry.create();

		registry.emplace<engine::NameComponent>(entity, "name");

		auto name_resolved = engine::get_indirect_value_or_ref(value, registry, entity);

		REQUIRE(name_resolved);

		auto as_string = name_resolved.try_cast<std::string>();

		REQUIRE(as_string);

		REQUIRE(*as_string == "test_name_a_b");
	}

	SECTION("Function call")
	{
		auto value = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest::fn(2)"),
			{ .allow_function_call_semantics = true }
		);

		REQUIRE(value);

		auto result = engine::get_indirect_value_or_ref(value);

		REQUIRE(result);

		auto as_instance = result.try_cast<engine::ReflectionTest>();

		REQUIRE(as_instance);

		REQUIRE(as_instance->x == 2);
		REQUIRE(as_instance->y == 4);
		REQUIRE(as_instance->z == 6);
	}

	SECTION("Member references")
	{
		auto value = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest::x"),
			{ .allow_member_references = true }
		);

		REQUIRE(value);

		engine::ReflectionTest instance = { 100, 300, 500 };
		
		auto result = engine::get_indirect_value_or_ref(value, entt::forward_as_meta(instance));

		REQUIRE(result);

		auto as_field = result.try_cast<std::int32_t>();

		REQUIRE(as_field);

		REQUIRE((*as_field) == 100);
	}

	SECTION("Nested type member references")
	{
		auto value = engine::meta_any_from_string
		(
			std::string_view("ReflectionTest::Nested::value"),
			{ .allow_member_references = true }
		);

		REQUIRE(value);

		engine::ReflectionTest instance = { 0, 0, 0, { 4.0 } };
		
		auto result = engine::get_indirect_value_or_ref(value, entt::forward_as_meta(instance.nested_value));

		REQUIRE(result);

		auto as_field = result.try_cast<float>();

		REQUIRE(as_field);

		REQUIRE((*as_field) >= 4.0);
	}

	SECTION("Integer")
	{
		auto value = engine::meta_any_from_string(std::string_view("3"));

		auto type = value.type();

		REQUIRE(value);
		REQUIRE(type == engine::resolve<std::int32_t>());

		auto* as_integer = value.try_cast<std::int32_t>();

		REQUIRE(as_integer);
		REQUIRE((*as_integer) == 3);
	}

	SECTION("Floating-point")
	{
		auto value = engine::meta_any_from_string(std::string_view("4.15"));

		auto type = value.type();

		REQUIRE(value);
		REQUIRE(type == engine::resolve<float>());

		auto* as_float = value.try_cast<float>();

		REQUIRE(as_float);
		REQUIRE((*as_float) >= 4.0f);
	}

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
}

TEST_CASE("engine::MetaValueOperation", "[engine:meta]")
{
	engine::reflect_all();

	SECTION("Plus operator")
	{
		auto value = engine::meta_any_from_string(std::string_view("3 + 5"), { .resolve_value_operations = true });

		auto* as_value_operation = value.try_cast<engine::MetaValueOperation>();

		REQUIRE(as_value_operation);

		auto result_raw = as_value_operation->get();

		REQUIRE(result_raw);
		REQUIRE(result_raw.type().is_integral());

		auto result = result_raw.cast<std::int32_t>();

		REQUIRE(result == 8);
	}

	SECTION("Order of operations")
	{
		auto value = engine::meta_any_from_string(std::string_view("1 + 10 * 5 + 1 * 3 * 4 / 2 - 10 + 1 + 1"), { .resolve_value_operations=true });

		auto* as_value_operation = value.try_cast<engine::MetaValueOperation>();

		REQUIRE(as_value_operation);

		auto result_raw = as_value_operation->get();

		REQUIRE(result_raw);
		REQUIRE(result_raw.type().is_integral());

		auto result = result_raw.cast<std::int32_t>();

		REQUIRE(result == 49);
	}

	SECTION("Parentheses")
	{
		auto value = engine::meta_any_from_string(std::string_view("(4 + 9) * 3"), { .resolve_value_operations = true }); // // "10 + 2 + (3 + 10) * 2"

		auto* as_value_operation = value.try_cast<engine::MetaValueOperation>();

		REQUIRE(as_value_operation);

		auto result_raw = as_value_operation->get();

		REQUIRE(result_raw);
		REQUIRE(result_raw.type().is_integral());

		auto result = result_raw.cast<std::int32_t>();

		REQUIRE(result == 39);
	}
}