#pragma once

#include <type_traits>
#include <optional>

#include "function_traits.hpp"
#include "member_traits.hpp"

namespace util
{
	namespace impl
	{
		template <class T, std::size_t=sizeof(T)>
		std::true_type is_complete_specialization_impl(T*);

		std::false_type is_complete_specialization_impl(...);

		template <typename T>
		struct is_complete_specialization : decltype(is_complete_specialization_impl(std::declval<T*>())) {};

		template <typename T> struct spec_test;
		template <> struct spec_test<int> {};

		static_assert(is_complete_specialization<spec_test<int>>::value);
		static_assert(!is_complete_specialization<spec_test<float>>::value);
	}

	template <typename T>
	using is_complete_specialization = impl::is_complete_specialization<T>;

	template <typename T>
	inline constexpr bool is_complete_specialization_v = is_complete_specialization<T>::value;

	template <typename T, template <typename...> typename Template>
	struct is_specialization : std::false_type {};

	template <template <typename...> typename Template, typename ...Args>
	struct is_specialization<Template<Args...>, Template> : std::true_type {};

	template <typename T, template <typename...> typename Template>
	inline constexpr bool is_specialization_v = is_specialization<T, Template>::value;

	template <typename ReturnType, typename Callable, typename ...Args>
	inline constexpr bool is_return_value = std::is_same_v<std::invoke_result_t<Callable, Args...>, ReturnType>;

	template<typename T>
	struct parent_type_from_member_pointer;

	template<typename Type, typename MemberType>
	struct parent_type_from_member_pointer<MemberType Type::*>
	{
		using type = Type;
		
		using parent_type = Type;
		using member_type = MemberType;
	};

	template<typename T>
	struct member_pointer_type;

	template<typename Type, typename MemberType>
	struct member_pointer_type<MemberType Type::*>
	{
		using type = MemberType;

		using parent_type = Type;
		using member_type = MemberType;
	};

	// Indicates whether type `T` is compatible with the `std::optional` template.
	template <typename T>
	struct is_optional_compatible : std::conjunction
	<
		std::negation<std::is_same<std::remove_cv_t<T>, std::nullopt_t>>,
		std::negation<std::is_same<std::remove_cv_t<T>, std::in_place_t>>,
		std::is_object<T>,
		std::is_destructible<T>,
		std::negation<std::is_array<T>>
	> {};

	template <typename T>
	inline constexpr bool is_optional_compatible_v = is_optional_compatible<T>::value;

	static_assert(is_optional_compatible_v<int>);
	static_assert(is_optional_compatible_v<decltype(static_cast<const char*>("."))>);
	static_assert(!is_optional_compatible_v<decltype(".")>);

	// Indicates whether type `T` is a C-string.
	// Detects string literals, const and non-const `char` pointers.
	template <typename T>
    struct is_c_str : std::disjunction
    <
        std::is_same<const char*, std::decay_t<T>>,
        std::is_same<char*, std::decay_t<T>>
    > {};

    template <typename T>
    inline constexpr bool is_c_str_v = is_c_str<T>::value;

    static_assert(is_c_str_v<decltype(".")>);
    static_assert(is_c_str_v<decltype(static_cast<const char*>("."))>);
    static_assert(is_c_str_v<decltype(const_cast<char*>(static_cast<const char*>(".")))>);
    static_assert(!is_c_str_v<decltype(100)>);
    static_assert(!is_c_str_v<decltype(nullptr)>);

	template <typename T>
    inline constexpr bool is_convertible_to_bool = (std::is_constructible_v<bool, decltype((std::declval<const T&>()))>); // (std::is_constructible_v<bool, const T&>) // (std::is_convertible_v<T, bool>);
}