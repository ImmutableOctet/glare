#pragma once

#include <engine/types.hpp>

#include <engine/world/physics/ground.hpp>

namespace engine
{
	class World;

	// Stores motion state information. (Boolean flags)
	struct MotionComponent
	{
		// Rules:
		//bool apply_gravity            : 1 = true;
		//bool apply_velocity           : 1 = true;
		bool align_to_ground          : 1 = true;
		bool reorient_in_air          : 1 = true;
		bool attach_to_dynamic_ground : 1 = true;

		// Motion flags:

		// State:
		bool on_ground   : 1 = false;

		// Indicates whether this entity is attached to another object. (e.g. dynamic ground)
		bool is_attached : 1 = false;

		//bool is_moving       : 1;
		//bool most_last_frame : 1;
	};
}