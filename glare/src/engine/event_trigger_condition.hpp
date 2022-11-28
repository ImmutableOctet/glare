#pragma once

#include "types.hpp"
#include "meta/types.hpp"

#include <string_view>

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

		bool condition_met(const MetaAny& instance) const;

		inline bool operator()(const MetaAny& instance) const
		{
			return condition_met(instance);
		}

		// `event_type` isn't actually needed, since we initialize this trigger's listener(s)
		// knowing the type, which is the only place where this would be used.
		// (Currently; may change in the future)
		// MetaType event_type;

		MetaSymbolID event_type_member; // entt::meta_data
		MetaAny comparison_value;
		ComparisonMethod comparison_method = ComparisonMethod::Equal;
	};
}