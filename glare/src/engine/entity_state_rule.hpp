#pragma once

#include "types.hpp"
#include "timer.hpp"
#include "meta/types.hpp"

#include "event_trigger_condition.hpp"
#include "entity_state_target.hpp"

#include <util/variant.hpp>
#include <util/small_vector.hpp>

#include <variant>
#include <optional>

namespace engine
{
	struct EntityStateTransitionRule
	{
		using TargetType = EntityStateTarget::TargetType;

		// The target (entity) this rule applies to.
		EntityStateTarget target;

		// The name of the state this `entity` will
		// transition to upon activation of `condition`.
		StringHash state_name;

		inline const TargetType& target_type() const
		{
			return target.type;
		}
	};

	struct EntityStateCommandRule
	{
		//MetaTypeDescriptor command;
	};

	using EntityStateAction = std::variant
	<
		EntityStateTransitionRule,
		EntityStateCommandRule
	>;

	struct EntityStateRule
	{
		using Action = EntityStateAction;

		std::optional<EventTriggerCondition> condition;
		std::optional<Timer::Duration> delay; // std::nullopt; // Timer::Duration::zero();

		Action action;
	};

	using EntityStateRuleCollection = util::small_vector<EntityStateRule, 4>;
}