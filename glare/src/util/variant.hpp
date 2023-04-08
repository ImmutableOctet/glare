#pragma once

#include <variant>
#include <utility>
#include <optional>
#include <type_traits>

namespace util
{
	// Helper templates (from cppreference):

	// Utility for the series of visitor functions passed into `std::visit`.
	template<class... Ts> struct visit_overloaded_t : Ts... { using Ts::operator()...; };

	// Explicit deduction guide. (Not needed from C++20 onward)
	template<class... Ts> visit_overloaded_t(Ts...)->visit_overloaded_t<Ts...>;

	// Similar to `std::visit`, but allows for a series of callbacks, rather than using `if constexpr` chains.
	// NOTE: You'll have to cover all possible variant types when declaring overloads.
	// NOTE: This version of `visit` takes `variant_obj` as the first argument, rather than as a trailing argument.
	template<typename variant_t, class... Callbacks>
	constexpr auto visit(variant_t&& variant_obj, Callbacks&&... overloads)
	{
		return std::visit(visit_overloaded_t<Callbacks...>(std::forward<Callbacks>(overloads)...), std::forward<variant_t>(variant_obj));
	}

	// Calls 'fn' with a value of type 'T' if 'v' contains said value.
	template <typename T, typename VariantType, typename FunctionType>
	bool peek_value(const VariantType& v, FunctionType fn)
	{
		if (const auto* value = std::get_if<T>(&v))
		{
			fn(*value);

			return true;
		}

		return false;
	}

	template <typename T, typename VariantType>
	T get_value(const VariantType& v, const T& default_value)
	{
		T value_out = default_value;

		peek_value<T>(v, [value_out](const auto& value) mutable
		{
			value_out = value;
		});

		return value_out;
	}

	template <typename T, typename VariantType>
	T get_value_safe(const VariantType& v, const T& default_value={})
	{
		return get_value(v, default_value);
	}

	template <typename T, typename VariantType>
	std::optional<T> get_optional(const VariantType& v)
	{
		T value_out;

		if (peek_value(v, [value_out](const auto& value) mutable { value_out = value; }))
		{
			return value_out;
		}
		else
		{
			return std::nullopt;
		}
	}

	// Retrieves a pointer from an `std::reference_wrapper<T>` value in `VariantType`.
	template <typename T, typename VariantType>
	T* get_if_wrapped(VariantType& v)
	{
		auto* wrapped = std::get_if<std::reference_wrapper<T>>(&v);

		return &(wrapped->get()); // &(static_cast<T&>(wrapped));
	}

	template<typename VT, typename T, std::size_t index = 0>
	constexpr std::size_t variant_index()
	{
		if constexpr (index == std::variant_size_v<VT>)
		{
			return index;
		}
		else if constexpr (std::is_same_v<std::variant_alternative_t<index, VT>, T>)
		{
			return index;
		}
		else
		{
			return variant_index<VT, T, index + 1>();
		}
	}

	template <typename T, typename variant_type>
	struct variant_contains : std::false_type
	{
		static_assert(std::integral_constant<T, false>::value, "`variant_type` must be a valid instantiation of `std::variant`.");
	};

	template <typename T, typename... variant_type_list>
	struct variant_contains<T, std::variant<variant_type_list...>>
		: std::disjunction<std::is_same<T, variant_type_list>...>
	{};

	template <typename variant_type, typename ...Ts>
	struct variant_contains_all_of : std::conjunction<variant_contains<Ts, variant_type>...> {};

	template <typename variant_type, typename ...Ts>
	struct variant_contains_any_of : std::disjunction<variant_contains<Ts, variant_type>...> {};

	template <typename variant_type, typename ...Ts>
	inline constexpr bool variant_contains_all_of_v = variant_contains_all_of<variant_type, Ts...>::value; // (variant_contains<Ts, variant_type>::value && ...);

	template <typename variant_type, typename ...Ts>
	inline constexpr bool variant_contains_any_of_v = variant_contains_any_of<variant_type, Ts...>::value; // (variant_contains<Ts, variant_type>::value || ...);

	// NOTE: Follows typename parameters in reverse order to allow for variadics.
	template <typename variant_type, typename ...Ts>
	inline constexpr bool variant_contains_v = variant_contains_all_of_v<variant_type, Ts...>; // (variant_contains<Ts, variant_type>::value && ...);

	static_assert(variant_contains_all_of_v<std::variant<int, short, float>, int, short, float>);
	static_assert(variant_contains_any_of_v<std::variant<int, float>, short, float>);
	static_assert(variant_contains_v<std::variant<int, float>, int, float>);

	template <typename variant_t>
	struct for_each_variant_type_impl;

	template <typename ...Types>
	struct for_each_variant_type_impl<std::variant<Types...>>
	{
		template <typename T>
		struct type
		{
			using underlying_t = T;
		};

		template <typename ...Args>
		void operator()(auto&& callback, Args&&... args)
		{
			(callback(type<Types>(), std::forward<Args>(args)...), ...);
		}
	};

	template <typename VariantType, typename ...Args>
	void for_each_variant_type(auto&& templated_lambda, Args&&... args)
	{
		for_each_variant_type_impl<VariantType>()
		(
			[&templated_lambda](auto&& symbolic, auto&&... args)
			{
				using symbolic_t = std::decay_t<decltype(symbolic)>;
				using underlying_t = typename symbolic_t::underlying_t;

				templated_lambda.template operator()<underlying_t>(std::forward<decltype(args)>(args)...);
			},

			std::forward<Args>(args)...
		);
	}
}