#pragma once

#include <engine/meta/types.hpp>

#include <type_traits>
#include <utility>

namespace engine
{
	namespace impl
	{
		template <typename EventType, typename PredicateType>
		constexpr auto generate_continuation_predicate_wrapper(PredicateType&& predicate)
		{
			return [predicate=std::forward<PredicateType>(predicate)](const Script& self, const MetaAny& opaque_value) -> bool
			{
				if constexpr (std::is_invocable_r_v<bool, PredicateType, const Script&, const EventType&>)
				{
					if (const auto as_event_type = opaque_value.try_cast<EventType>())
					{
						return predicate(self, *as_event_type);
					}

					return false;
				}
				else if constexpr (std::is_invocable_r_v<bool, PredicateType, const EventType&>)
				{
					if (const auto as_event_type = opaque_value.try_cast<EventType>())
					{
						return predicate(*as_event_type);
					}

					return false;
				}
				else if constexpr (std::is_invocable_r_v<bool, PredicateType, const Script&, const MetaAny&>)
				{
					return predicate(self, opaque_value);
				}
				else if constexpr (std::is_invocable_r_v<bool, PredicateType, const MetaAny&>)
				{
					return predicate(opaque_value);
				}
				else if constexpr (std::is_invocable_r_v<bool, PredicateType>)
				{
					return predicate();
				}
				else if constexpr (std::is_invocable_v<PredicateType>)
				{
					if constexpr (util::is_convertible_to_bool<std::invoke_result_t<PredicateType>>)
					{
						return static_cast<bool>(predicate());
					}
					else
					{
						predicate();

						return true;
					}
				}
				else
				{
					return false;
				}
			};
		}

		template <typename PredicateType>
		constexpr auto generate_as_is_predicate_wrapper(PredicateType&& predicate)
		{
			return predicate;
		}

		template <typename ScriptType, typename PredicateRefType>
		void register_continuation_predicate_ref(ScriptType& script, PredicateRefType predicate_ref)
		{
			script.set_yield_continuation_predicate([&predicate_ref](auto&&... args) { return predicate_ref(std::forward<decltype(args)>(args)...); });
		}
	}
}