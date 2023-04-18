#pragma once

/*
* Based on example code found here by 'T.C.':
* https://stackoverflow.com/questions/27879815/c11-get-type-of-first-second-etc-argument-similar-to-result-of
*/

#include <tuple>
#include <type_traits>
#include <functional>

namespace util
{
    template <typename T>
    struct function_traits : function_traits<decltype(&T::operator())> {};

    // Partial specialization for function-type.
    template <typename R, typename... Args>
    struct function_traits<R(Args...)>
    {
        using return_type = R;
        using argument_types = std::tuple<Args...>;

        inline static constexpr bool is_dynamic_member_function = false;
    };

    // Partial specialization for function-pointer.
    template <typename R, typename... Args>
    struct function_traits<R(*)(Args...)>
    {
        using return_type = R;
        using argument_types = std::tuple<Args...>;

        inline static constexpr bool is_dynamic_member_function = false;
    };

    // Partial specialization for `std::function`.
    template <typename R, typename... Args>
    struct function_traits<std::function<R(Args...)>>
    {
        using return_type = R;
        using argument_types = std::tuple<Args...>;

        inline static constexpr bool is_dynamic_member_function = false;
    };

    // Partial specialization for pointer to dynamic member-function.
    template <typename T, typename R, typename... Args>
    struct function_traits<R(T::*)(Args...)>
    {
        using self_type = T;
        using self_type_ref = T&;
        using return_type = R;
        using argument_types = std::tuple<Args...>;

        inline static constexpr bool is_dynamic_member_function = true;
    };

    // Partial specialization for pointer to const dynamic member-function.
    template <typename T, typename R, typename... Args>
    struct function_traits<R(T::*)(Args...) const>
    {
        using self_type = T;
        using self_type_ref = const T&;
        using return_type = R;
        using argument_types = std::tuple<Args...>;

        inline static constexpr bool is_dynamic_member_function = true;
    };

    template <typename T>
    using first_argument_type = typename std::tuple_element<0, typename function_traits<T>::argument_types>::type;
}