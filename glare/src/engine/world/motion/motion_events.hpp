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

		// The surface (ground) we contacted.
		Ground surface;

		// A length-vector representing the movement
		// that caused contact with the ground.
		inline math::Vector landing_vector() const
		{
			return surface.impact_velocity;
		}

		// The entity that collided with the ground.
		// NOTE: Opposite of `surface`'s definition.
		inline Entity entity() const
		{
			return surface.collision.a;
		}

		// The ground entity we're contacting.
		inline Entity ground() const
		{
			return surface.entity();
		}

		// The world-space position of the ground we contacted.
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

		// The surface (ground) we're leaving from.
		Ground surface;

		math::Vector entity_position;
		math::Vector escape_vector;

		// The entity that is leaving the ground.
		// NOTE: Opposite of `surface`'s definition.
		inline Entity entity() const
		{
			return surface.contacting_entity();
		}

		// The ground entity we're leaving.
		inline Entity ground() const
		{
			return surface.entity();
		}
	};
}