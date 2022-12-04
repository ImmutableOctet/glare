#pragma once

#include "types.hpp"
#include "meta/types.hpp"

#include <string_view>
#include <utility>

namespace engine
{
	// TODO: Implement compound conditions.
	// i.e. Check against clauses/multiple data-members of a type.

	struct EventTriggerCondition
	{
		enum class ComparisonMethod : std::uint8_t
		{
			Equal,
			NotEqual,

			// Currently unsupported:
			/*
			LessThan,
			LessThanOrEqualTo,

			GreaterThan,
			GreaterThanOrEqualTo,
			*/
		};
		
		static ComparisonMethod get_comparison_method(std::string_view comparison_operator);

		bool condition_met(const MetaAny& event_instance, Registry& registry, Entity entity) const;
		bool condition_met(const MetaAny& event_instance) const;
		bool condition_met(const MetaAny& event_instance, const MetaAny& comparison_value) const;

		template <typename ...Args>
		inline bool operator()(Args&&... args) const
		{
			return condition_met(std::forward<Args>(args)...);
		}

		MetaAny get_member_value(const MetaAny& event_instance) const;

		inline const MetaAny& get_comparison_value() const { return comparison_value; }

		// `event_type` isn't actually needed, since we initialize this trigger's listener(s)
		// knowing the type, which is the only place where this would be used.
		// (Currently; may change in the future)
		// MetaType event_type;

		MetaSymbolID event_type_member;
		MetaAny comparison_value;

		ComparisonMethod comparison_method = ComparisonMethod::Equal;
	};
}