#pragma once

#include <engine/meta/types.hpp>

#include <util/overloads.hpp>
#include <util/lambda.hpp>

#include <utility>
#include <type_traits>
#include <cstddef>

namespace engine
{
	// Generates a wrapper function for `lambda_instance`, allowing a captureless
    // lambda to be used in place of a function-pointer. (Similar to `+[]` syntax)
    template <auto lambda_instance>
    constexpr auto make_overload()
    {
        return util::lambda_as_function_ptr<lambda_instance>();
    }

    template <auto fn_ptr, auto generic_impl_lambda, std::size_t min_args=0>
    constexpr auto make_overloads(auto type, MetaTypeID function_id)
    {
        util::generate_function_overloads_ex
        <
            fn_ptr,
            decltype(generic_impl_lambda), // generic_impl_lambda,
            min_args
        >
        (
            [&type, function_id]<auto overload_fn_ptr>()
            {
                type = type.func<overload_fn_ptr>(function_id);
            }
        );

        return type;
    }

    template
    <
        typename T, typename trait,
        bool is_const=false, bool is_noexcept=false,
        typename policy=entt::as_is_t
    >
    bool reflect_function(auto& type, auto function_id)
    {
        if constexpr (trait::value)
        {
            if constexpr (trait::template ptr<is_const, is_noexcept>)
            {
                type = type.template func
                <
                    trait::template ptr<is_const, is_noexcept>,
                    policy
                >(function_id);

                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    template <typename T, bool is_const, bool is_noexcept, typename policy, typename ...traits>
    auto& reflect_any_function(auto& type, auto function_id) // constexpr
    {
        (reflect_function<T, traits, is_const, is_noexcept, policy>(type, function_id) || ...);

        return type;
    }

    template <typename T, bool is_const, bool is_noexcept, typename policy, typename ...traits>
    auto& reflect_any_function(auto& type, auto function_id, bool& result_out) // constexpr
    {
        if ((reflect_function<T, traits, is_const, is_noexcept, policy>(type, function_id) || ...))
        {
            result_out = true;
        }

        return type;
    }
}