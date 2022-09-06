#pragma once

#include "types.hpp"

namespace engine
{
	class World;

	// TODO: Rename this component to something more fitting.
	struct PhysicsComponent
	{
		using Flags = MotionFlags;

		struct MotionState
		{
			math::Vector prev_position = {};
			math::Vector velocity = {};

			float gravity_mag = 0.0f; // 1.0f; // 0.01f;
			float deceleration = 0.0f;
		};

		PhysicsComponent(Flags motion_flags=(Flags::StandardMovement), MotionState motion_state=MotionState());

		Flags flags;
		MotionState motion;

		inline float gravity() const { return motion.gravity_mag; }

		inline bool apply_gravity()  const { return (flags & Flags::ApplyGravity);  }
		inline bool apply_velocity() const { return (flags & Flags::ApplyVelocity); }
	};

	Entity attach_physics(World& world, Entity entity, PhysicsComponent::Flags flags, PhysicsComponent::MotionState motion_state={});
}