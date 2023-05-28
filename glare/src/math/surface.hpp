#pragma once

#include "types.hpp"
#include "common.hpp"

namespace math
{
	// Utility function; provides the cross-product (third direction-vector) of `normal` and `forward`.
	// See also: `get_surface_slope`.
	inline math::Vector get_surface_forward
	(
		const math::Vector& normal,
		const math::Vector& forward={0.0f, 0.0f, -1.0f}
	)
	{
		return cross(normal, forward);
	}

	// Returns the slope of the surface (`normal`)
	// from the approach angle (`angle`).
	inline float get_surface_slope
	(
		const math::Vector& normal,
		const math::Vector& angle,
		const math::Vector& forward={0.0f, 0.0f, -1.0f}
	)
	{
		auto adjacent = get_surface_forward(normal, forward);

		return dot(angle, adjacent);
	}
}