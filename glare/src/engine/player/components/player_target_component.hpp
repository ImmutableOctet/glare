#pragma once

#include <engine/player/types.hpp>

namespace engine
{
	// Indicates a designated target player by index.
	// 
	// This is useful for things like control-based event triggers,
	// where you only want to capture a specific player's input events, etc.
	struct PlayerTargetComponent
	{
		PlayerIndex player_index;
	};
}