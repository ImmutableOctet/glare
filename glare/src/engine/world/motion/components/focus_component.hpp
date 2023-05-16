#pragma once

#include <engine/types.hpp>
#include <math/types.hpp>

namespace engine
{
	struct FocusComponent
	{
		// The target entity this entity is focussing on (looking at).
		Entity target = null;

		// An offset from the target entity's position.
		math::Vector focus_offset = {};

		// The speed at which this entity tracks the position of the `target` entity.
		float tracking_speed = 0.2f;
	};
}