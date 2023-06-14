#pragma once

#include <type_traits>
#include <optional>
#include <array>
#include <iterator>

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

		template <typename>
		struct array_size_trait;

		template <typename T, std::size_t N>
		struct array_size_trait<std::array<T, N>>
		{
			inline static constexpr std::size_t value = N;
			//inline static constexpr std::size_t size = N;
		};

		struct constructible_from_float_or_int
		{
			constructible_from_float_or_int(float);
			constructible_from_float_or_int(int);
		};

		struct constructible_from_float
		{
			constructible_from_float(float);
		};
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

	template <typename ArrayType>
	inline constexpr auto array_size = impl::array_size_trait<ArrayType>::value;

	static_assert(array_size<std::array<int, 3>> == 3);

	template <typename T, typename=void>
	struct is_iterable : std::false_type {};

	template <typename T>
	struct is_iterable
	<
		T,
		
		std::void_t
		<
			decltype(std::begin(std::declval<T&>())),
			decltype(std::end(std::declval<T&>()))
		>
	> : std::true_type {};

	template <typename T>
	inline constexpr bool is_iterable_v = is_iterable<T>::value;

	template <typename T, typename=void>
	struct is_const_iterable : std::false_type {};

	template <typename T>
	struct is_const_iterable
	<
		T,

		std::void_t
		<
			decltype(std::cbegin(std::declval<const T&>())),
			decltype(std::cend(std::declval<const T&>()))
		>
	> : std::true_type {};

	template <typename T>
	inline constexpr bool is_const_iterable_v = is_const_iterable<T>::value;

	template <typename T, typename=void>
	struct is_iterator : std::false_type {};

	template <typename T>
	struct is_iterator
	<
		T,
		
		std::void_t
		<
			typename std::iterator_traits<T>::iterator_category
		>
	> : std::true_type {};

	template <typename T>
	inline constexpr bool is_iterator_v = is_iterator<T>::value;

	template <bool apply_decay, typename T, typename... AllowedProxies>
	struct constructor_proxy_ex
	{
		template <typename AttemptedProxyType>
		inline static constexpr bool is_compatible =
		(
			(
				(apply_decay)
					? (std::is_same_v<std::decay_t<AttemptedProxyType>, std::decay_t<T>>)
					: (std::is_same_v<AttemptedProxyType, T>)
			)
			||
			(
				(sizeof...(AllowedProxies) > 0)
				&&
				(
					(apply_decay)
						? ((std::is_same_v<std::decay_t<AttemptedProxyType>, std::decay_t<AllowedProxies>>) || ...)
						: ((std::is_same_v<AttemptedProxyType, AllowedProxies>) || ...)
				)
			)
		);

		inline static constexpr bool value = (std::is_convertible_v<AllowedProxies, T> || ...);

		template<typename AttemptedProxyType, typename=std::enable_if_t<is_compatible<AttemptedProxyType>>>
		operator AttemptedProxyType() const
		{
			return AttemptedProxyType {};
		}
	};

	template <typename T, typename... AllowedProxies>
	struct constructor_proxy : constructor_proxy_ex<true, T, AllowedProxies...> {};

	//template <typename T>
	//using disable_narrowing = constructor_proxy<T>;

	template <typename T, typename InputType>
	using is_constructible_without_conversion = std::is_constructible<T, constructor_proxy_ex<false, InputType>>;

	template <typename T, typename InputType>
	inline constexpr bool is_constructible_without_conversion_v = is_constructible_without_conversion<T, InputType>::value;

	template <typename T, typename InputType, typename... OtherInputTypes>
	using is_constructible_without_conversion_or = std::is_constructible<T, constructor_proxy_ex<false, InputType, OtherInputTypes...>>;

	template <typename T, typename InputType, typename... OtherInputTypes>
	inline constexpr bool is_constructible_without_conversion_or_v = is_constructible_without_conversion_or<T, InputType, OtherInputTypes...>::value;

	template <typename T, typename... InputTypes>
	using is_constructible_without_conversion_and = std::conjunction<std::is_constructible<T, constructor_proxy_ex<false, InputTypes>>...>;

	template <typename T, typename... InputTypes>
	inline constexpr bool is_constructible_without_conversion_and_v = is_constructible_without_conversion_and<T, InputTypes...>::value;

	static_assert(is_constructible_without_conversion_v<impl::constructible_from_float, float>);
	static_assert(!is_constructible_without_conversion_v<impl::constructible_from_float, double>);
	static_assert(!is_constructible_without_conversion_v<impl::constructible_from_float, long double>);

	static_assert(is_constructible_without_conversion_or_v<impl::constructible_from_float, float, double, int>);
	static_assert(is_constructible_without_conversion_and_v<impl::constructible_from_float_or_int, float, int>);
	static_assert(!is_constructible_without_conversion_and_v<impl::constructible_from_float_or_int, float, double, int>);
}