#pragma once

#include <math/types.hpp>

namespace engine
{
	// TODO: Move from `camera` submodule and rename to `FreeLookComponent`.
	struct CameraFreeLookComponent
	{
		math::Vector2D movement_speed = { 0.04f, 0.04f };
	};
}