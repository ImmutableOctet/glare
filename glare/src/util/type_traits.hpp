#pragma once

#include <type_traits>
#include <optional>
#include <array>
#include <iterator>
#include <functional>
#include <utility>

#include "function_traits.hpp"
#include "member_traits.hpp"
#include "empty.hpp"

namespace util
{
	namespace impl
	{
		template <typename T, std::size_t=sizeof(T)>
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

		template <template <typename...> typename C>
		void get_base_template_type_impl(...);

		template <template <typename...> typename Template, typename... TemplateArgs>
		Template<TemplateArgs...>* get_base_template_type_impl(const Template<TemplateArgs...>*);

		template <template <typename...> typename C>
		std::false_type is_base_template_impl(...);

		template <template <typename...> typename Template, typename... TemplateArgs>
		std::true_type is_base_template_impl(const Template<TemplateArgs...>*);

		template <template <typename...> typename Template, typename T>
		struct is_base_template : decltype(is_base_template_impl<Template>(std::declval<T*>())) {};

		template <bool result, template <typename...> typename Template, typename T>
		struct base_template_type : is_base_template<Template, T> {};

		template <template <typename...> typename Template, typename T>
		struct base_template_type<true, Template, T> : is_base_template<Template, T>
		{
			using type = std::remove_pointer_t<decltype(get_base_template_type_impl<Template>(std::declval<T*>()))>;
		};
	}

	template <typename T>
	using is_complete_specialization = impl::is_complete_specialization<T>;

	template <typename T>
	inline constexpr bool is_complete_specialization_v = is_complete_specialization<T>::value;

	template <template <typename...> typename Template, typename T>
	using is_base_template = impl::is_base_template<Template, T>;

	template <template <typename...> typename Template, typename T>
	inline constexpr bool is_base_template_v = is_base_template<Template, T>::value;

	template <template <typename...> typename Template, typename T>
	using base_template_type = impl::base_template_type<impl::is_base_template<Template, T>::value, Template, T>;

	template <template <typename...> typename Template, typename T>
	using base_template_type_t = base_template_type<Template, T>::type;

	template <template <typename...> typename Template, typename T>
	inline constexpr bool base_template_type_v = base_template_type<Template, T>::value;

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

	template <>
	inline constexpr bool is_convertible_to_bool<void> = false;

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

	template <typename T>
	using integral_or_size = std::conditional<std::is_integral_v<std::remove_reference_t<T>>, std::remove_reference_t<T>, std::size_t>;

	template <typename T>
	using integral_or_size_t = typename integral_or_size<T>::type;

	template <typename T>
	struct is_const_ptr : std::is_const<std::remove_pointer_t<T>> {};

	template <typename T>
	inline constexpr bool is_const_ptr_v = is_const_ptr<T>::value;

    template <typename T>
    inline constexpr bool is_standard_function_type_v = is_specialization_v<T, std::function>;

    template <typename T>
    struct always_false_t : std::false_type {};

    template <typename T>
    inline constexpr bool always_false_v = always_false_t<T>::value;

    template <typename T>
    inline constexpr bool always_false = always_false_v<T>;

	template <typename A, typename B>
	using equality_comparison_result_t = decltype(std::declval<A>() == std::declval<B>());

	template <typename A, typename B, typename=std::void_t<>>
	struct is_equality_comparable : std::false_type {};

	template <typename A, typename B>
	struct is_equality_comparable<A, B, std::void_t<equality_comparison_result_t<A, B>>>
		: std::is_same<equality_comparison_result_t<A, B>, bool> {};

	template <typename A, typename B>
	inline constexpr bool is_equality_comparable_v = is_equality_comparable<A, B>::value;

	template <typename T, typename=void>
	struct is_defined : std::false_type {};

	template <typename T>
	struct is_defined
	<
		T,

		std::enable_if_t
		<
			(
				(std::is_object<T>::value)
				&&
				(!std::is_pointer<T>::value)
				&&
				(sizeof(T) > 0)
			)
		>
	> : std::true_type
	{
		using type = T;
	};

	template <typename T>
	using is_defined_t = is_defined<T>::type;

	template <typename T>
	inline constexpr bool is_defined_v = is_defined<T>::value;

	template <bool condition, typename T>
	struct true_or_empty : std::conditional<condition, T, util::empty_type<T>> {};

	template <bool condition, typename T>
	using true_or_empty_t = true_or_empty<condition, T>::type;

	template <typename T>
	struct defined_or_empty : true_or_empty<is_defined_v<T>, T> {};

	template <typename T>
	using defined_or_empty_t = defined_or_empty<T>::type;

	template <typename T>
	using decay_rvalue_t = std::conditional_t
	<
		std::is_rvalue_reference_v<T>,
		std::decay_t<T>,
		T
	>;
}