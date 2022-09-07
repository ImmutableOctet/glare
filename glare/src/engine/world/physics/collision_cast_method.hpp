#pragma once

#include <types.hpp>
//#include <engine/types.hpp>

namespace engine
{
	struct CollisionComponent;

	enum class CollisionCastMethod : std::uint8_t
	{
		None,

		RayCast,
		ConvexCast,

		// TODO: Look into this option and how it differs from `ConvexCast`.
		// (see: `btKinematicClosestNotMeConvexResultCallback`)
		//ConvexKinematicCast
	};
}