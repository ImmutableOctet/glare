#pragma once

#include <engine/types.hpp>

namespace engine
{
	// Indicates which state an entity is currently in.
	struct StateComponent
	{
		EntityStateIndex state_index;
		EntityStateIndex prev_state_index;

		//EntityStateHash state_id;
	};

	using EntityStateComponent = StateComponent;
}