#pragma once

#include <engine/types.hpp>

namespace engine
{
	class World;

	struct MotionComponent
	{
		Entity ground = null;

		math::Vector velocity = {};

		float ground_deceleration = 0.0f;

		// Motion flags:
		
		// Regulatory:
		bool apply_gravity     : 1 = true;
		bool apply_velocity    : 1 = true;
		bool align_to_surfaces : 1 = true;
		bool reorient_in_air   : 1 = true;

		// State:
		bool on_ground         : 1 = false;
		bool ground_is_static  : 1 = true;

		//bool is_moving       : 1;
		//bool most_last_frame : 1;
	};
}