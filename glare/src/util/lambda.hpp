#pragma once

#include "tuple.hpp"
#include "function_traits.hpp"
#include "functor.hpp"

#include <tuple>
#include <utility>

namespace util
{
	// Generates a wrapper function for `lambda_instance`, allowing a captureless
	// lambda to be used in place of a function-pointer. (Similar to `+[]` syntax)
    template <typename Functor>
    constexpr auto lambda_type_as_function_ptr()
    {
        using ReturnType = typename util::function_traits<decltype(&Functor::operator())>::return_type;
        using ArgumentTuple = typename util::function_traits<decltype(&Functor::operator())>::argument_types;

        using TemplateInput = decltype
        (
            std::tuple_cat
            (
                std::declval<std::tuple<Functor>>(),
                std::declval<std::tuple<ReturnType>>(),
                std::declval<ArgumentTuple>()
            )
        );
    
        return &(instantiate_template_with_tuple<wrap_functor, TemplateInput>::type::execute);
    }

	template <auto lambda_instance>
    constexpr auto lambda_as_function_ptr()
    {
        using Functor = decltype(lambda_instance);

		return lambda_type_as_function_ptr<Functor>();
	}
}