#pragma once

#include "types.hpp"

namespace engine
{
	class World;

	struct MotionComponent
	{
		math::Vector velocity;

		float ground_deceleration = 0.0f;

		// Motion flags:
		
		// Regulatory:
		bool apply_gravity     : 1 = true;
		bool apply_velocity    : 1 = true;

		// State:
		bool on_ground         : 1 = false;

		//bool is_moving       : 1;
		//bool most_last_frame : 1;
	};
}