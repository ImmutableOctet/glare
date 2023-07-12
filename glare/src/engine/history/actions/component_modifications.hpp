#pragma once

#include "entity_action.hpp"

#include <engine/meta/types.hpp>

namespace engine::history
{
	struct ComponentModifications : public EntityAction
	{
		using HistoryComponentID = MetaTypeID;

		// A list of component types attached to this entity that have been modified.
		util::small_vector<HistoryComponentID, 4> components_modified;
	};
}