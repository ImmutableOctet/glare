#pragma once

#include "awaiters.hpp"
#include "control_flow_token.hpp"
#include "conditional_yield_request.hpp"
#include "script_fiber_response.hpp"

#include <util/variant.hpp>
#include <util/function_traits.hpp>
#include <util/type_traits.hpp>

#include <type_traits>
#include <utility>

namespace engine
{
	namespace impl
	{
		template <typename ValidatedPredicateType>
		auto script_predicate_co_await_impl(ValidatedPredicateType&& predicate)
		{
			return ConditionalRequestAwaiter
			{
				ConditionalYieldRequest
				{
					ControlFlowToken::UntilWoken,

					std::forward<ValidatedPredicateType>(predicate)
				}
			};
		}

		template <typename FiberResponseVariantType>
		auto script_fiber_response_co_await_impl(FiberResponseVariantType&& request)
		{
			auto immediate_request = ImmediateYieldRequest
			{
				std::forward<FiberResponseVariantType>(request)
			};
			
			// Since the underlying request's result won't change (rvalue/temporary),
			// execute the parameterless call operator, forcing a `true` result status:
			if constexpr ((std::is_rvalue_reference_v<FiberResponseVariantType>) || (!std::is_reference_v<FiberResponseVariantType>))
			{
				//immediate_request();
			}

			return ImmediateRequestAwaiter
			{
				std::move(immediate_request)
			};
		}
	}

	namespace script
	{
		template
		<
			typename RequestType,

			typename std::enable_if
			<
				// NOTE: This approach is not (currently) possible due to limitations with mixing
				// variadic template template parameters and non-type template parameters.
				// i.e. template <typename ..., auto...> doesn't work as a template parameter.
				//(util::is_base_template_v<ConditionalYieldRequestImpl, std::remove_cvref_t<RequestType>>),

				(std::is_base_of_v<RequestAwaiterWrappable, std::remove_cvref_t<RequestType>>),
			
				//(util::is_base_template_v<ConditionalEventYieldRequest, std::remove_cvref_t<RequestType>>),
			
				int
			>::type=0
		>
		auto operator co_await(RequestType&& request)
		{
			return ConditionalRequestAwaiter
			{
				std::move(request) // std::forward<RequestType>
			};
		}

		template
		<
			typename BooleanPredicate,

			typename std::enable_if
			<
				(
					(!std::is_base_of_v<RequestAwaiterWrappable, std::remove_cvref_t<BooleanPredicate>>)
					&&
					(!util::variant_contains_v<ScriptFiberResponse, std::remove_cvref_t<BooleanPredicate>>)
					&&
					((std::is_invocable_v<BooleanPredicate>) && (util::is_convertible_to_bool<std::invoke_result_t<BooleanPredicate>>))
				),
				
				int
			>::type=0
		>
		auto operator co_await(BooleanPredicate&& predicate)
		{
			return impl::script_predicate_co_await_impl(std::forward<BooleanPredicate>(predicate));
		}

		template
		<
			typename BooleanPredicate,

			typename std::enable_if
			<
				(
					(!std::is_base_of_v<RequestAwaiterWrappable, std::remove_cvref_t<BooleanPredicate>>)
					&&
					((std::is_invocable_v<BooleanPredicate, const Script&>) && (util::is_convertible_to_bool<std::invoke_result_t<BooleanPredicate, const Script&>>))
				),
				
				int
			>::type=0
		>
		auto operator co_await(BooleanPredicate&& predicate)
		{
			return impl::script_predicate_co_await_impl(std::forward<BooleanPredicate>(predicate));
		}

		template
		<
			typename BooleanPredicate,

			typename std::enable_if
			<
				(
					(!std::is_base_of_v<RequestAwaiterWrappable, std::remove_cvref_t<BooleanPredicate>>)
					&&
					((std::is_invocable_v<BooleanPredicate, Script&>) && (!std::is_invocable_v<BooleanPredicate, const Script&>) && (util::is_convertible_to_bool<std::invoke_result_t<BooleanPredicate, Script&>>))
				),
				
				int
			>::type=0
		>
		auto operator co_await(BooleanPredicate&& predicate)
		{
			return impl::script_predicate_co_await_impl(std::forward<BooleanPredicate>(predicate));
		}

		template
		<
			typename BooleanPredicate,

			typename std::enable_if
			<
				(
					(!std::is_base_of_v<RequestAwaiterWrappable, std::remove_cvref_t<BooleanPredicate>>)
					&&
					(util::is_convertible_to_bool<typename util::function_traits<BooleanPredicate>::return_type>)
				),
				int
			>::type=0,

			typename std::enable_if
			<
				(std::tuple_size_v<typename util::function_traits<BooleanPredicate>::argument_types> > 0),
				int
			>::type=0,

			typename std::enable_if
			<
				(
					(!std::is_same_v<Script, std::decay_t<std::tuple_element_t<0, typename util::function_traits<BooleanPredicate>::argument_types>>>)
					&&
					(std::is_base_of_v<Script, std::remove_reference_t<std::tuple_element_t<0, typename util::function_traits<BooleanPredicate>::argument_types>>>)
				),
				
				int
			>::type=0
		>
		auto operator co_await(BooleanPredicate&& predicate)
		{
			using DerivedScriptRefType = std::tuple_element_t<0, typename util::function_traits<BooleanPredicate>::argument_types>;
			using BaseScriptRefType = std::conditional_t<std::is_const_v<std::remove_reference_t<DerivedScriptRefType>>, const Script&, Script&>;

			return impl::script_predicate_co_await_impl
			(
				[predicate=std::forward<BooleanPredicate>(predicate)](BaseScriptRefType script)
				{
					return static_cast<bool>(predicate(static_cast<DerivedScriptRefType>(script)));
				}
			);
		}

		template
		<
			typename RequestType,

			typename std::enable_if
			<
				(std::is_same_v<std::remove_cvref_t<RequestType>, ScriptFiberResponse>),
			
				int
			>::type=0
		>
		auto operator co_await(RequestType&& request)
		{
			//return impl::script_fiber_response_co_await_impl(std::forward<RequestType>(request));
		}

		template
		<
			typename RequestType,

			typename std::enable_if
			<
				(util::variant_contains_v<ScriptFiberResponse, std::remove_cvref_t<RequestType>>),
			
				int
			>::type=0
		>
		auto operator co_await(RequestType&& request)
		{
			return impl::script_fiber_response_co_await_impl(ScriptFiberResponse { std::forward<RequestType>(request) });
		}
	}
}