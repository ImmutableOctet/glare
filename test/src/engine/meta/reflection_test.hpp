#pragma once

#include <engine/types.hpp>
//#include <engine/meta/types.hpp>

#include <string>
#include <optional>

#include <cstddef>

namespace engine
{
	struct ReflectionTest
	{
		struct Nested
		{
			inline static Nested make_nested(float value)
			{
				return { value };
			}

			inline float get() const { return value; }

			inline Nested(float value={}) : value(value) {}

			float value;

			bool operator==(const Nested&) const noexcept = default;
			bool operator!=(const Nested&) const noexcept = default;
		};

		std::int32_t x = {};
		std::int32_t y = {};
		std::int32_t z = {};

		Nested nested_value = {};

		inline ReflectionTest(std::int32_t x, std::int32_t y, std::int32_t z, const Nested& nested_value={})
			: x(x), y(y), z(z), nested_value(nested_value)
		{}

		inline ReflectionTest() {}

		/*
		inline ReflectionTest(const ReflectionTest& r)
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

		inline std::int32_t get_property_x() const
		{
			return x;
		}

		inline void set_property_x(std::int32_t value)
		{
			x = value;
		}

		inline static ReflectionTest fn(std::int32_t value=1)
		{
			return { value*1, value*2, value*3 };
		}

		inline ReflectionTest operator+(const ReflectionTest& instance) const
		{
			return { (x + instance.x), (y + instance.y), (z + instance.z), { nested_value.value + instance.nested_value.value } };
		}

		inline std::int32_t const_method_test() const { return y; }
		inline std::int32_t non_const_method_test() { return z; }

		inline std::int32_t const_opaque_method() const
		{
			const auto sum_value = static_cast<std::int32_t>(*this);

			return (sum_value * sum_value);
		}

		inline std::int32_t non_const_opaque_method()
		{
			x++;
			y++;
			z++;

			return static_cast<std::int32_t>(*this);
		}

		inline static std::int32_t function_with_context(Registry& registry, Entity entity)
		{
			return static_cast<std::int32_t>(entity);
		}

		inline Entity method_with_context(Registry& registry, Entity entity)
		{
			return entity;
		}

		inline Nested& get_nested() { return nested_value; }

		inline static std::optional<ReflectionTest> get_optional_value(bool provide_value)
		{
			if (provide_value)
			{
				return ReflectionTest { 10, 20, 30, { 40.0f  } };
			}

			return std::nullopt;
		}

		inline static std::string overloaded_function(std::int32_t value)
		{
			return "integer";
		}

		inline static std::string overloaded_function(const std::string& value)
		{
			return "string";
		}
	};

	struct PFRReflectionTest
	{
		struct NestedA
		{
			int a;
		};

		struct NestedB
		{
			struct NestedInNestedB
			{
				char nested_b;
			};

			NestedInNestedB nested_in_nested_b;

			int b;
		};

		struct NestedC
		{
			struct NestedInNestedC // : NestedB::NestedInNestedB // <-- Inherited type are not currently supported by Boost PFR.
			{
				float nested_c;
			};

			NestedInNestedC nested_in_nested_c;

			short c;
		};

		NestedA nested_a;
		NestedB nested_b;
		NestedC nested_c;
	};

	template <typename T = void> void reflect();

	extern template void reflect<ReflectionTest>();
}