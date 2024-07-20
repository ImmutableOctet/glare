#pragma once

#include <util/type_traits.hpp>
#include <util/member_traits.hpp>

#include <type_traits>

namespace engine
{
	namespace script
	{
		namespace traits
		{
			GENERATE_HAS_METHOD_TRAIT(get_script);
			
			GENERATE_HAS_METHOD_TRAIT(yield_value);

			GENERATE_HAS_METHOD_TRAIT(await_suspend);
			GENERATE_HAS_METHOD_TRAIT(await_resume);
			GENERATE_HAS_METHOD_TRAIT(await_ready);
			GENERATE_HAS_METHOD_TRAIT(initial_suspend);
			GENERATE_HAS_METHOD_TRAIT(final_suspend);

			GENERATE_HAS_TYPE_MEMBER_TRAIT(CoreScriptType);

			template <typename method_trait>
			inline constexpr bool has_any_qualifier_permutation =
			(
				(method_trait::template ptr<false, false>)
				||
				(method_trait::template ptr<true, false>)
				||
				(method_trait::template ptr<false, true>)
				||
				(method_trait::template ptr<true, true>)
			);

			template <typename method_trait, bool is_noexcept=false>
			inline constexpr bool has_any_const_permutation =
			(
				(method_trait::template ptr<false, is_noexcept>)
				||
				(method_trait::template ptr<true, is_noexcept>)
			);

			template <typename method_trait, bool is_const=false>
			inline constexpr bool has_any_noexcept_permutation =
			(
				(method_trait::template ptr<is_const, false>)
				||
				(method_trait::template ptr<is_const, true>)
			);

			template <typename EventType, typename PredicateType>
			struct is_continuation_predicate_wrappable : std::integral_constant
			<
				bool,

				(
					(std::is_invocable_r_v<bool, PredicateType, const Script&, const EventType&>)
					||
					(std::is_invocable_r_v<bool, PredicateType, const EventType&>)
					||
					(std::is_invocable_r_v<bool, PredicateType, const Script&, const MetaAny&>)
					||
					// Disabled for now; ambiguity issues.
					//(std::is_invocable_r_v<bool, PredicateType, const MetaAny&>)
					//||
					(std::is_invocable_r_v<bool, PredicateType>)
					||
					(std::is_invocable_v<PredicateType>)
				)
			> {};

			template <typename EventType, typename PredicateType>
			inline constexpr bool is_continuation_predicate_wrappable_v = is_continuation_predicate_wrappable<EventType, PredicateType>::value;

			template <typename SelfType, typename ScriptType>
			inline constexpr bool is_compatible_script_type_v = std::is_base_of_v
			<
				typename std::decay_t<SelfType>::CoreScriptType,

				std::decay_t<ScriptType>
			>;

			template <typename T>
			using decay_temporary_t = util::decay_rvalue_t<T>;
		}
	}
}