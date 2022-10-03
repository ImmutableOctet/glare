#pragma once

#include <engine/types.hpp>

#include "ground.hpp"

namespace engine
{
	class World;

	struct MotionRules
	{
		bool apply_gravity            : 1 = true;
		bool apply_velocity           : 1 = true;
		bool reorient_in_air          : 1 = true;
		bool align_to_ground          : 1 = true;
		bool attach_to_dynamic_ground : 1 = true;
	};

	struct MotionComponent : public MotionRules
	{
		MotionComponent() = default;
		MotionComponent(const MotionComponent&) = default;
		MotionComponent(MotionComponent&&) noexcept = default;

		inline MotionComponent(const MotionRules& rules)
			: MotionRules(rules) {}

		inline void set_rules(const MotionRules& rules)
		{
			MotionRules::operator=(rules);
		}

		MotionComponent& operator=(const MotionComponent&) = default;
		MotionComponent& operator=(MotionComponent&&) noexcept = default;

		// Motion flags:

		// State:
		bool on_ground   : 1 = false;

		// Indicates whether this entity is attached to another object. (e.g. dynamic ground)
		bool is_attached : 1 = false;

		//bool is_moving       : 1;
		//bool most_last_frame : 1;

		// The most recent ground contact.
		// If `on_ground` is true, this is the ground the entity is currently standing on.
		// If `on_ground` is false, this represents the previous ground the entity contacted.
		Ground ground;

		math::Vector velocity = {};

		float ground_deceleration = 0.0f;
	};
}