#pragma once

#include <engine/input/events.hpp>

namespace engine
{
	// Used for camera-relative analog input.
	struct OnRelativeAnalogInput : OnAnalogInput
	{
		// The world-space direction of the input.
		math::Vector3D direction = {};

		// The entity (camera) that generated this event.
		Entity source = null;
	};
}