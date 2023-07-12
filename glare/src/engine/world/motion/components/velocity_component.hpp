#pragma once

#include <math/types.hpp>
#include <math/common.hpp>

namespace engine
{
	struct VelocityComponent
	{
		math::Vector velocity = {};

		inline float speed() const
		{
			return math::length(velocity);
		}

		inline math::Vector direction() const
		{
			return math::normalize(velocity);
		}

		inline explicit operator math::Vector() const
		{
			return velocity;
		}
	};
}