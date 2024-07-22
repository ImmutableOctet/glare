#pragma once

#include "entity_action.hpp"

#include <engine/meta/types.hpp>

#include <util/small_vector.hpp>

namespace engine::history
{
	struct ComponentCreations : public EntityAction
	{
		using LifetimeComponentID = MetaTypeID;

		// A list of component types that have been created and attached to this entity.
		util::small_vector<LifetimeComponentID, 4> components_created;
	};
}