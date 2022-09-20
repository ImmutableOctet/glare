#pragma once

#include <engine/types.hpp>
#include <math/types.hpp>

#include "ground.hpp"

namespace engine
{
	// Triggered once an entity (that had previously been in the air) lands on the ground.
	// This event is triggered by entities with a `MotionComponent` attached.
	struct OnAirToGround
	{
		//Entity entity;
		Ground surface;

		inline math::Vector landing_vector() const
		{
			return surface.impact_velocity;
		}

		inline Entity entity() const
		{
			return surface.collision.a;
		}

		inline Entity ground() const
		{
			return surface.entity();
		}

		inline math::Vector ground_position() const
		{
			return surface.collision.position;
		}
	};

	// Triggered once an entity (that had previously been on the ground) enters the air.
	// This event is triggered by entities with a `MotionComponent` attached.
	struct OnGroundToAir
	{
		//Entity entity;
		Ground surface;

		math::Vector entity_position;
		math::Vector escape_vector;

		inline Entity entity() const
		{
			return surface.contacting_entity();
		}

		inline Entity ground() const
		{
			return surface.entity();
		}
	};
}