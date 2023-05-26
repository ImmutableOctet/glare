#pragma once

#include <cstddef>

namespace engine
{
	enum class EntityThreadCadence : std::uint8_t
	{
		Update,
		Fixed,

		// Functions equivalently to a `MultiControlBlock`.
		// This allows for an alternative `cadence` instruction syntax.
		// (Useful for dynamically determining execution rate)
		Realtime,

		// Aliases:
		
		// Alias to `Realtime`. (Equivalent to `MultiControlBlock`)
		Multi = Realtime,

		FixedUpdate = Fixed,
		FixedUpdateBased = FixedUpdate,

		UpdateBased = Update,
		FrameDriven = UpdateBased,

		Default = Update, // Realtime // Fixed,
	};
}