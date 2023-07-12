#pragma once

#include "entity_action.hpp"

#include <engine/meta/types.hpp>

namespace engine::history
{
	struct ComponentDestructions : public EntityAction
	{
		using LifetimeComponentID = MetaTypeID;

		// A list of component types that have been detached and destroyed from this entity.
		util::small_vector<LifetimeComponentID, 4> components_destroyed;
	};
}