#pragma once

#include <engine/types.hpp>

#include "ground.hpp"

namespace engine
{
	class World;

	struct MotionComponent
	{
		// The most recent ground contact.
		// If `on_ground` is true, this is the ground the entity is currently standing on.
		// If `on_ground` is false, this represents the previous ground the entity contacted.
		Ground ground;

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

		//bool is_moving       : 1;
		//bool most_last_frame : 1;
	};
}