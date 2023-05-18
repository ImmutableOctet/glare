#pragma once

#include <math/types.hpp>

namespace engine
{
	struct DirectionComponent
	{
		// The target direction the entity should face.
		math::Vector direction;

		// The speed at which the entity turns towards `direction`.
		float turn_speed = 0.2f;

		// If enabled, the `x` value of the target `direction` is ignored.
		bool ignore_x : 1 = false;

		// If enabled, the `y` value of the target `direction` is ignored.
		bool ignore_y : 1 = false;

		// If enabled, the `z` value of the target `direction` is ignored.
		bool ignore_z : 1 = false;

		// Reflection wrappers:
		inline bool get_ignore_x() const { return ignore_x; }
		inline bool get_ignore_y() const { return ignore_y; }
		inline bool get_ignore_z() const { return ignore_z; }

		inline void set_ignore_x(bool value) { ignore_x = value; }
		inline void set_ignore_y(bool value) { ignore_y = value; }
		inline void set_ignore_z(bool value) { ignore_z = value; }
	};
}