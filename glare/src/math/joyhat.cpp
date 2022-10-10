#include "joyhat.hpp"

namespace math
{
	Vector2D joyhat(bool up, bool down, bool left, bool right)
	{
		Vector2D direction = {};

		if (up)
		{
			direction.y += 1.0f;
		}

		if (down)
		{
			direction.y -= 1.0f;
		}

		if (left)
		{
			direction.x -= 1.0f;
		}

		if (right)
		{
			direction.x += 1.0f;
		}

		return direction;
	}

	float joyhat_angle(bool up, bool down, bool left, bool right)
	{
		return direction_to_angle(joyhat(up, down, left, right));
	}
}