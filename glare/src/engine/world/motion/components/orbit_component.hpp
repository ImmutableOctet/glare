#pragma once

#include <engine/types.hpp>

#include <engine/input/analog.hpp>

#include <math/types.hpp>

namespace engine
{
	struct OrbitComponent
	{
		// The speed at which the camera moves, based on analog input.
		math::Vector3D movement_speed = { 0.04f, 0.04f, 2.0f };

		// The current distance from the target entity.
		float distance = 20.0f;

		// The closest distance the camera can be to the target entity.
		float min_distance = 8.0f;

		// The furthest distance the camera can be from the target entity.
		float max_distance = 60.0f;
	};
}