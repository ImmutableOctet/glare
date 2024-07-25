#pragma once

#include <engine/player/types.hpp>

namespace engine
{
	// Indicates which player index this entity represents.
	// (Used to mark an entity as the base/parent entity of a player object)
	struct PlayerComponent
	{
		PlayerIndex player_index;
	};
}