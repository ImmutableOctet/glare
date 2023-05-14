#pragma once

#include <math/types.hpp>

namespace engine
{
	struct CameraFreeLookComponent
	{
		math::Vector2D movement_speed = { 0.04f, 0.04f };
	};
}