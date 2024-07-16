#pragma once

#include <tuple>
#include <type_traits>
#include <functional>

namespace util
{
    namespace impl
    {
        template <typename T, typename R, typename... Args>
        inline constexpr auto call_operator_ptr_v = static_cast<R(T::*)(Args...)>(&T::operator());

        template <typename T, typename R, typename... Args>
        using call_operator_ptr_t = decltype(static_cast<R(T::*)(Args...)>(&T::operator())); // decltype(call_operator_ptr_v<T, R, Args...>);

        template <typename T, typename R, typename... Args>
        inline constexpr auto noexcept_call_operator_ptr_v = static_cast<R(T::*)(Args...) noexcept>(&T::operator());

        template <typename T, typename R, typename... Args>
        using noexcept_call_operator_ptr_t = decltype(static_cast<R(T::*)(Args...) noexcept>(&T::operator())); // decltype(noexcept_call_operator_ptr_v<T, R, Args...>);

        template <typename T, typename R, typename... Args>
        inline constexpr auto const_call_operator_ptr_v = static_cast<R(T::*)(Args...) const>(&T::operator());

        template <typename T, typename R, typename... Args>
        using const_call_operator_ptr_t = decltype(static_cast<R(T::*)(Args...) const>(&T::operator())); // decltype(const_call_operator_ptr_v<T, R, Args...>);

        template <typename T, typename R, typename... Args>
        inline constexpr auto const_noexcept_call_operator_ptr_v = static_cast<R(T::*)(Args...) const noexcept>(&T::operator());

        template <typename T, typename R, typename... Args>
        using const_noexcept_call_operator_ptr_t = decltype(static_cast<R(T::*)(Args...) const noexcept>(&T::operator())); // decltype(const_noexcept_call_operator_ptr_v<T, R, Args...>);

        template <bool is_const, bool is_noexcept, typename T, typename R, typename... Args>
        constexpr auto resolve_call_operator_ptr()
        {
            if constexpr (is_const)
            {
                if constexpr (is_noexcept)
                {
                    return const_noexcept_call_operator_ptr_v<T, R, Args...>;
                }
                else
                {
                    return const_call_operator_ptr_v<T, R, Args...>;
                }
            }
            else
            {
                if constexpr (is_noexcept)
                {
                    return noexcept_call_operator_ptr_v<T, R, Args...>;
                }
                else
                {
                    return call_operator_ptr_v<T, R, Args...>;
                }
            }
        }
    }

    template <typename T, typename R, typename... Args>
    inline constexpr auto call_operator_ptr_v = impl::call_operator_ptr_v<T, R, Args...>;

    template <typename T, typename R, typename... Args>
    using call_operator_ptr_t = impl::call_operator_ptr_t<T, R, Args...>;

    template <bool is_const, bool is_noexcept, typename T, typename R, typename... Args>
    inline constexpr auto call_operator_ptr_ex_v = impl::resolve_call_operator_ptr<is_const, is_noexcept, T, R, Args...>();

    template <bool is_const, bool is_noexcept, typename T, typename R, typename... Args>
    using call_operator_ptr_ex_t = decltype(call_operator_ptr_ex_v<is_const, is_noexcept, T, R, Args...>);

    template <typename T>
    inline constexpr bool is_standalone_function_v = std::is_function_v<std::remove_pointer_t<std::remove_reference_t<T>>>;

    template <typename T>
    inline constexpr bool is_member_function_pointer_v = std::is_member_function_pointer_v<T>;

    // Type trait used to indicate whether `T`'s call operator has a single overload.
    // i.e. `&T::operator()` is unambiguous.
    template <typename T, typename=void>
    struct has_one_call_operator : std::false_type {};

    template <typename T>
    struct has_one_call_operator
    <
        T,
        std::void_t<decltype(&T::operator())>
    > : std::true_type
    {
        inline static constexpr auto pointer_value = (&T::operator());

        using pointer_type = decltype(pointer_value); // decltype(&T::operator());
    };

    // Indicates whether `T`'s call operator has a single overload.
    template <typename T>
    inline constexpr bool has_one_call_operator_v = has_one_call_operator<T>::value;

    template <typename T, typename=void>
    struct function_traits {};

    // Specialization for function-type:
    template <typename R, typename... Args>
    struct function_traits<R(Args...), std::void_t<R(Args...)>>
    {
        using return_type = R;
        using argument_types = std::tuple<Args...>;

        inline static constexpr bool is_standalone_function_pointer = true;
        inline static constexpr bool is_any_function_pointer        = true;
        inline static constexpr bool is_member_function             = false;
        inline static constexpr bool is_standard_function           = false;
    };

    // Specialization for pointer to function:
    template <typename R, typename... Args>
    struct function_traits<R(*)(Args...)> : function_traits<R(Args...)> {};

    // Specialization for reference to function:
    template <typename R, typename... Args>
    struct function_traits<R(&)(Args...)> : function_traits<R(Args...)> {};

    // Specialization for `std::function`:
    template <typename R, typename... Args>
    struct function_traits<std::function<R(Args...)>>
    {
        using return_type = R;
        using argument_types = std::tuple<Args...>;

        inline static constexpr bool is_standalone_function_pointer = false;
        inline static constexpr bool is_any_function_pointer        = false;
        inline static constexpr bool is_member_function             = false;
        inline static constexpr bool is_standard_function           = true;
    };

    // Specialization for pointer to dynamic member-function:
    template <typename T, typename R, typename... Args>
    struct function_traits<R(T::*)(Args...)>
    {
        using self_type = T;
        using self_type_ref = T&;
        using return_type = R;
        using argument_types = std::tuple<Args...>;

        inline static constexpr bool is_standalone_function_pointer = false;
        inline static constexpr bool is_any_function_pointer        = true;
        inline static constexpr bool is_member_function             = true;
        inline static constexpr bool is_standard_function           = false;
    };

    template <typename T, typename R, typename... Args>
    struct function_traits<R(T::*)(Args...) noexcept>
        : function_traits<R(T::*)(Args...)> {};

    // Specialization for pointer to const dynamic member-function:
    template <typename T, typename R, typename... Args>
    struct function_traits<R(T::*)(Args...) const>
    {
        using self_type = T;
        using self_type_ref = const T&;
        using return_type = R;
        using argument_types = std::tuple<Args...>;

        inline static constexpr bool is_standalone_function_pointer = false;
        inline static constexpr bool is_any_function_pointer        = true;
        inline static constexpr bool is_member_function             = true;
        inline static constexpr bool is_standard_function           = false;
    };

    template <typename T, typename R, typename... Args>
    struct function_traits<R(T::*)(Args...) const noexcept>
        : function_traits<R(T::*)(Args...) const> {};

    // Specialization for types with a single/unambiguous call operator:
    template <typename T>
    struct function_traits<T, std::void_t<decltype(&T::operator())>> : function_traits<decltype(&T::operator())> {};

    // Utilities for retrieving traits for call operators where the function signature is known:
    template <bool is_const, bool is_noexcept, typename T, typename R, typename... Args>
    using overloaded_call_operator_traits_ex = function_traits<call_operator_ptr_ex_t<is_const, is_noexcept, T, R, Args...>>;

    template <typename T, typename R, typename... Args>
    using overloaded_call_operator_traits = function_traits<call_operator_ptr_t<T, R, Args...>>;

    // Simplified type retrieval:
    template <typename T>
    using first_argument_type = typename std::tuple_element<0, typename function_traits<T>::argument_types>::type;

    template <typename T>
    using second_argument_type = typename std::tuple_element<1, typename function_traits<T>::argument_types>::type;
}