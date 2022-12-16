#pragma once

#include "types.hpp"

//#include <engine/meta/types.hpp>
#include <engine/meta/meta_type_descriptor.hpp>
#include <engine/meta/meta_description.hpp>

#include <variant>

namespace engine
{
	// Indicates a state for the targeted entity.
	struct EntityStateTransitionAction
	{
		// The name of the state this `entity` will
		// transition to upon activation of `condition`.
		StringHash state_name;
	};

	// Stores values to be forwarded to a triggered `Command` type.
	struct EntityStateCommandAction
	{
		using CommandContent = MetaTypeDescriptor;

		CommandContent command;
	};

	// Describes an 'update' operation for an entity,
	// modifying and/or reinitializing its components accordingly.
	struct EntityStateUpdateAction
	{
		using Components = MetaDescription;

		Components updated_components;

		/*
			NOTES:
			
			* The entity referenced by this target is
			relative to the active target for the rule.
			
			i.e. `self` would only resolve to the rule's affected entity
			if the state's target also happens to be the same entity.

			* This works in conjunction with `EntityStateRule::target` to
			allow for modification of multiple target entities.

			* During the processing step, the `target` key is currently
			reserved as a 'special symbol' for this feature.
		*/
		EntityTarget target_entity;
	};

	using EntityStateAction = std::variant
	<
		EntityStateTransitionAction,
		EntityStateCommandAction,
		EntityStateUpdateAction
	>;
}