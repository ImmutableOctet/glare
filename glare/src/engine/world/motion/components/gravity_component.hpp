#pragma once

#include <math/types.hpp>

namespace engine
{
	// Entities with this component will have gravity applied.
	// Gravity is defined by the `World` object.
	// 
	// NOTE:
	// Some objects may still have gravity applied without this component,
	// if their gravity calculation is sourced from an external system
	// (i.e. dynamic physics objects)
	struct GravityComponent
	{
		float intensity = 1.0f;

		inline math::Vector get_vector(const math::Vector& gravity, float delta) const
		{
			return (gravity * intensity * delta);
		}
	};
}