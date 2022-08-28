#pragma once

#include <engine/types.hpp>
#include <math/math.hpp>

namespace engine
{
	enum class MotionFlags : std::uint8_t
	{
		None = 0,

		ApplyGravity = (1 << 0),
		ApplyVelocity = (1 << 2),

		StandardMovement = (ApplyGravity|ApplyVelocity),
	};

	FLAG_ENUM(std::uint8_t, MotionFlags);
}