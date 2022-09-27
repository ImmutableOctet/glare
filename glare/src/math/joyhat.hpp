#pragma once

#include "types.hpp"
#include "math.hpp"

namespace math
{
	inline Vector2D joyhat(bool up, bool down, bool left, bool right)
	{
		Vector2D direction = {};

		if (up)
		{
			direction.y = 1.0f;
		}
		else if (down)
		{
			direction.y = -1.0f;
		}

		if (left)
		{
			direction.x = -1.0f;
		}
		else if (right)
		{
			direction.x = 1.0f;
		}

		return direction;
	}

	inline float joyhat_angle(bool up, bool down, bool left, bool right)
	{
		return direction_to_angle(joyhat(up, down, left, right));
	}
}