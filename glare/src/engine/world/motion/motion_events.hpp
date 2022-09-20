#pragma once

#include <engine/types.hpp>
#include <math/types.hpp>

namespace engine
{
	// Triggered once an entity (that had previously been in the air) lands on the ground.
	// This event is triggered by entities with a `MotionComponent` attached.
	struct OnAirToGround
	{
		Entity entity;
		Entity ground;

		math::Vector entity_position;
		math::Vector ground_position;
		math::Vector landing_vector;
	};

	// Triggered once an entity (that had previously been on the ground) enters the air.
	// This event is triggered by entities with a `MotionComponent` attached.
	struct OnGroundToAir
	{
		Entity entity;
		Entity ground;

		math::Vector entity_position;
		math::Vector escape_vector;
	};
}