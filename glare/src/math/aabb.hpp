#pragma once

#include "types.hpp"

#include <tuple>

namespace math
{
	struct AABB
	{
		Vector min;
		Vector max;

		// Dimensional lengths for each axis. (`max` - `min`)
		Vector dim_lengths() const;

		// Vector-length of dimensional lengths result. (Outer/encapsulating radius)
		float length() const;

		// Average of all three length properties. ('Inner' radius)
		float average_length() const;

		// Returns a point at the center of the AABB.
		Vector get_center_point() const;
	};
}