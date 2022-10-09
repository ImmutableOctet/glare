#pragma once

#include "types.hpp"
#include "math.hpp"

namespace math
{
	Vector2D joyhat(bool up, bool down, bool left, bool right);
	float joyhat_angle(bool up, bool down, bool left, bool right);
}