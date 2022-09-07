#pragma once

#include <types.hpp>
//#include <engine/types.hpp>

namespace engine
{
	enum class CollisionCastMethod : std::uint8_t
	{
		None,
		RayCast,
		ConvexCast,
		ConvexKinematicCast,

		Default=ConvexCast
	};
}