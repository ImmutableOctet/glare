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
    // primary template.
    template<class T>
    struct function_traits : function_traits<decltype(&T::operator())> {};

    // partial specialization for function type
    template<class R, class... Args>
    struct function_traits<R(Args...)> {
        using result_type = R;
        using argument_types = std::tuple<Args...>;
    };

    // partial specialization for function pointer
    template<class R, class... Args>
    struct function_traits<R(*)(Args...)> {
        using result_type = R;
        using argument_types = std::tuple<Args...>;
    };

    // partial specialization for std::function
    template<class R, class... Args>
    struct function_traits<std::function<R(Args...)>> {
        using result_type = R;
        using argument_types = std::tuple<Args...>;
    };

    // partial specialization for pointer-to-member-function (i.e., operator()'s)
    template<class T, class R, class... Args>
    struct function_traits<R(T::*)(Args...)> {
        using result_type = R;
        using argument_types = std::tuple<Args...>;
    };

    template<class T, class R, class... Args>
    struct function_traits<R(T::*)(Args...) const> {
        using result_type = R;
        using argument_types = std::tuple<Args...>;
    };

    template<class T>
    using first_argument_type = typename std::tuple_element<0, typename function_traits<T>::argument_types>::type;
}