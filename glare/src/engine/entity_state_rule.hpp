#pragma once

#include "types.hpp"
#include "timer.hpp"

#include "meta/types.hpp"
#include "meta/meta_type_descriptor.hpp"

#include "event_trigger_condition.hpp"
#include "entity_target.hpp"

#include <util/variant.hpp>
#include <util/small_vector.hpp>

#include <variant>
#include <optional>

namespace engine
{
	struct EntityStateTransitionRule
	{
		// The name of the state this `entity` will
		// transition to upon activation of `condition`.
		StringHash state_name;
	};

	struct EntityStateCommandRule
	{
		using CommandContent = MetaTypeDescriptor;

		CommandContent command;
	};

	using EntityStateAction = std::variant
	<
		EntityStateTransitionRule,
		EntityStateCommandRule
	>;

	struct EntityStateRule
	{
		using Action = EntityStateAction;
		using TargetType = EntityTarget::TargetType;

		std::optional<EventTriggerCondition> condition;
		std::optional<Timer::Duration> delay; // std::nullopt; // Timer::Duration::zero();

		// The target (entity) this rule applies to.
		EntityTarget target;

		Action action;

		inline Entity resolve_target(Registry& registry, Entity source=null) const
		{
			return target.resolve(registry, source);
		}

		inline const TargetType& target_type() const
		{
			return target.type;
		}
	};

	using EntityStateRuleCollection = util::small_vector<EntityStateRule, 4>;
}