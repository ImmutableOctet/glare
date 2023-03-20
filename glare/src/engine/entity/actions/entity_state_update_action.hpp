#pragma once

#include <engine/entity/entity_target.hpp>

#include <engine/entity/meta_description.hpp>
//#include <engine/meta/meta_type_descriptor.hpp>

#include <memory>

namespace engine
{
	// Describes an 'update' operation for an entity,
	// modifying and/or reinitializing its components accordingly.
	//
	// TODO: Look into whether it's worth changing this to use shared storage mechanism.
	struct EntityStateUpdateAction
	{
		using ComponentStore = MetaDescription;

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

		ComponentStore updated_components = {};

		bool operator==(const EntityStateUpdateAction&) const noexcept = default;
		bool operator!=(const EntityStateUpdateAction&) const noexcept = default;
	};
}