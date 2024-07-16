#pragma once

#include "function_traits.hpp"
#include "type_traits.hpp"
//#include "lambda.hpp"

#include <type_traits>
#include <tuple>
#include <utility>

namespace util
{
	namespace impl
	{
		// Alternative implementation to using `lambda_type_as_function_ptr`.
		template <typename generic_lambda_t, typename... Args>
		decltype(auto) generate_function_overloads_impl_forwarding_function(Args... args) // Args&&...
		{
			auto generic_impl = generic_lambda_t();

			if constexpr (std::is_same_v<decltype(generic_impl.operator()(std::declval<Args>()...)), void>) // std::forward<Args>(args)... // generic_impl.template operator()
			{
				generic_impl.operator()(std::forward<Args>(args)...); // generic_impl.template operator()
			}
			else
			{
				return generic_impl.operator()(std::forward<Args>(args)...); // generic_impl.template operator
			}
		}

		template <typename generic_lambda_t, typename functions_out_lambda_t, typename generic_lambda_args, std::size_t min_args, bool is_member_function>
		constexpr void generate_function_overloads_impl(functions_out_lambda_t&& output_instance)
		{
			deplete_tuple_types<generic_lambda_args, min_args>
            (
                [&output_instance]<typename... Ts>()
                {
                    // Not reliable, since technically the unevaluable portion would be inside of the lambda itself.
                    if constexpr (std::is_invocable_v<generic_lambda_t, Ts...>) // generic_lambda_t::operator()<Ts...>
                    {
						/*
						// Alternative implementation (leverages generic lambda):
						constexpr auto function_impl = [](Ts... args)
						{
							auto generic_impl = generic_lambda_t();

							if constexpr(std::is_same_v<decltype(generic_impl.template operator()(std::forward<decltype(args)>(args)...)), void>)
							{
								generic_impl.template operator()(std::forward<decltype(args)>(args)...);
							}
							else
							{
								return generic_impl.template operator()(std::forward<decltype(args)>(args)...);
							}
						};

						using function_impl_t = decltype(function_impl);

						constexpr auto function_ptr_out = lambda_type_as_function_ptr<function_impl_t>(); // lambda_as_function_ptr
						*/

						constexpr auto function_ptr_out = &generate_function_overloads_impl_forwarding_function<generic_lambda_t, Ts...>;

						output_instance.operator()<function_ptr_out>(); // template operator()<function_ptr_out>();
                    }
                }
            );
		}
	}

	// Generates wrapper function pointers for `fn_ptr` from `generic_lambda`, calling `functions_out_lambda` with the results.
	// This is useful for cases where function-pointers to different default-argument configurations are required.
	template <auto fn_ptr, typename generic_lambda_t, std::size_t min_args, typename functions_out_lambda_t>
	constexpr void generate_function_overloads_ex(functions_out_lambda_t&& functions_out_lambda)
	{
		using traits = util::function_traits<decltype(fn_ptr)>;
		using argument_types = typename traits::argument_types;
    
		if constexpr (traits::is_member_function)
		{
			using self_type_ref = typename traits::self_type_ref;

			using generic_lambda_args = decltype
			(
				std::tuple_cat
				(
					std::declval<std::tuple<self_type_ref>>(), // std::tuple<std::reference_wrapper<std::remove_reference_t<self_type_ref>>>
					std::declval<argument_types>()
				)
			);

			return impl::generate_function_overloads_impl
			<
				generic_lambda_t, functions_out_lambda_t,
				generic_lambda_args,

				// One always required for 'self' type.
				(min_args + 1),
                true
			>(std::forward<functions_out_lambda_t>(functions_out_lambda));
		}
		else
		{
			using generic_lambda_args = argument_types;

			return impl::generate_function_overloads_impl
			<
				generic_lambda_t, functions_out_lambda_t,
				generic_lambda_args,
				min_args,
                false
			>(std::forward<functions_out_lambda_t>(functions_out_lambda));
		}
	}

	template <auto fn_ptr, auto generic_lambda, std::size_t min_args, typename functions_out_lambda_t>
	constexpr void generate_function_overloads(functions_out_lambda_t&& functions_out_lambda)
	{
		using generic_lambda_t = decltype(generic_lambda);

		generate_function_overloads_ex<fn_ptr, generic_lambda_t, min_args, functions_out_lambda_t>(std::forward<functions_out_lambda_t>(functions_out_lambda));
	}
}