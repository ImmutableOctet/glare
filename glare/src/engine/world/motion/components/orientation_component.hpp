#pragma once

#include <math/types.hpp>

namespace engine
{
	struct OrientationComponent
	{
		// Constructs a `OrientationComponent` pointed towards the world-space direction specified.
		static OrientationComponent from_direction(const math::Vector& direction, float turn_speed=0.2f);

		// The orientation the entity should apply.
		math::Quaternion orientation;

		// The speed at which the entity applies `orientation`.
		float turn_speed = 0.2f;

		void set_direction(const math::Vector& direction);
		math::Vector get_direction() const;
	};
}