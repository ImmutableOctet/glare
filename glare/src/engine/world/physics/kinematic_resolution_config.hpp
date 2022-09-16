#pragma once

#include "collision_cast_method.hpp"

#include <variant>

namespace engine
{
	struct CollisionComponent;

	// Collision resolution information for kinematic objects.
	// This is used to supply data to 
	struct KinematicResolutionConfig
	{
		// Uses an AABB for the shape/body.
		// (based on Bullet's `getAabb` and related functions)
		// NOTE: Bullet sometimes pads the size of AABBs.
		struct AABBType {};

		/*
			Uses sphere-radius of object/shape.
			This is based on Bullet's `getBoundingSphere` command.
			
			NOTE:
			
			Because this is based on Bullet's logic, it uses AABBs under the hood,
			which effectively makes this: [sphere -> cube -> sphere] when using a spherical collision shape.
			
			The effect of this is a radius/size large enough to hold a cube, that itself is
			large enough to hold the original sphere shape, making the result unexpectedly large.

			As a workaround, I have also implemented `InnerSphereType`, which should give the
			expected result for spherical collision objects.
		*/
		struct SphereType {};

		// Uses inner sphere-radius of object/shape.
		// This is based on `CollisionComponent::get_inner_radius`.
		struct InnerSphereType {};

		// Allows the user to specify an exact size.
		struct SizeType
		{
			protected:
				float half_size;

				// Alternate implementation:
				//float size;
			public:
				inline SizeType(float size):
					//size(size),
					half_size(size * 0.5f)
				{}

				//SizeType(const SizeType&) = default;
				//SizeType(SizeType&&) = default;

				inline float get_size() const { return (half_size * 2.0f); }
				inline float get_half_size() const { return half_size; }

				// Alternate implementation:
				//float get_size() const { return size; }
				//inline float get_half_size() const { return (size / 2.0f); }
		};

		using SizeConfig = std::variant<std::monostate, AABBType, SphereType, InnerSphereType, SizeType>;

		CollisionCastMethod cast_method = CollisionCastMethod::None;
		SizeConfig size = std::monostate{};

		// If true, this object can influence the position of other kinematic objects.
		// The affected object must also be accepting influences. (see `accepts_influence`)
		// NOTE: Kinematic influences are only applicable to objects that can collide with one another.
		bool is_influencer     = false;

		// If true, this object can be kinematically influenced by another object.
		bool accepts_influence = false;

		// If true, this object is subject to intersection resolution outside of casting.
		bool resolve_intersections = true;

		float get_size(const CollisionComponent& collision) const;
		float get_half_size(const CollisionComponent& collision) const;
	};
}