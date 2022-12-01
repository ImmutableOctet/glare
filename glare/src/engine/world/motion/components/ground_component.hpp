#pragma once

#include <engine/world/physics/ground.hpp>

namespace engine
{
	// Stores details about the ground the entity is touching/last touched.
	struct GroundComponent
	{
		// NOTE: May remove this in favor of a statically defined (or `World` / `PhysicsSystem` defined) detection threshold.
		float detection_threshold = 0.65f; // 0.5f; // (Dot-product value)

		// The most recent ground contact.
		// 
		// Component interop notes:
		// If `MotionComponent::on_ground` is true, this is the ground the entity is currently standing on.
		// If `MotionComponent::on_ground` is false, this represents the previous ground the entity contacted.
		Ground ground;

		inline Entity entity() const { return ground.entity(); }

		inline bool get_on_ground() const { return ground.get_is_contacted(); }
		inline void set_on_ground(bool value) { ground.set_is_contacted(value); }

		inline bool on_ground() const { return get_on_ground(); }
		inline explicit operator bool() const { return get_on_ground(); }

		inline float get_detection_threshold() const { return detection_threshold; } // { return 0.65f; }
	};
}