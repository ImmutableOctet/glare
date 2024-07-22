#pragma once

#include <math/types.hpp>

#include <graphics/math_types.hpp>

namespace engine
{
	// Global values applied to a `World`.
	struct WorldProperties
	{
		graphics::ColorRGB ambient_light = { 0.5f, 0.5f, 0.5f };
		math::Vector gravity = { 0.0f, -1.0f, 0.0f };
	};
}