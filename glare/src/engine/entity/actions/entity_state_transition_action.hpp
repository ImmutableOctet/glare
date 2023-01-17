#pragma once

#include <engine/types.hpp>

namespace engine
{
	// Indicates a state for the targeted entity.
	struct EntityStateTransitionAction
	{
		// The name of the state this `entity` will
		// transition to upon activation of `condition`.
		StringHash state_name;
	};
}