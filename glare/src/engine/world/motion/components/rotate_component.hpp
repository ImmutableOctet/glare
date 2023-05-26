#pragma once

#include <engine/types.hpp>

#include <math/types.hpp>

namespace engine
{
	struct Transform;

	struct RotateComponent
	{
		// Constructs a `RotateComponent` from the world-space direction specified.
		static RotateComponent from_direction(const math::Vector& direction, float turn_speed=0.2f, bool use_local_rotation=false);

		// The relative orientation with which the entity should rotate.
		math::Quaternion relative_orientation;

		// The speed at which the entity turns.
		float turn_speed = 0.2f;

		// If enabled, rotation will be performed in local rotation-space.
		// If disabled, rotation will be performed in world-space.
		bool use_local_rotation : 1 = false;

		math::Quaternion get_next_basis(const Transform& tform, float delta=1.0f) const;

		// Sets `relative_orientation` to the direction specified.
		// NOTE: This overrides/removes the `roll` of the final orientation.
		void set_direction(const math::Vector& direction);

		// Retrieves the next world-space direction `self` will point towards, after this rotation is applied.
		math::Vector get_next_direction(Entity self, Registry& registry, float delta=1.0f) const;

		// Retrieves the next world-space direction `tform` will point towards, after this rotation is applied.
		math::Vector get_next_direction(const Transform& tform, float delta=1.0f) const;

		// Reflection wrappers:
		inline math::Vector get_direction(Entity self, Registry& registry) const
		{
			return get_next_direction(self, registry);
		}

		inline bool get_use_local_rotation() const { return use_local_rotation; }
		inline void set_use_local_rotation(bool value) { use_local_rotation = value; }
	};
}