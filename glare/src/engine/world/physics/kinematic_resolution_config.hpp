#pragma once

#include "collision_cast_method.hpp"

#include <variant>

namespace engine
{
	// Collision resolution information for kinematic objects.
	// This is used to supply data to 
	struct KinematicResolutionConfig
	{
		// Uses an AABB for the shape/body.
		// (based on Bullet's `getAabb` and related functions)
		// NOTE: Bullet sometimes pads the size of AABBs.
		struct AABBType {};

		// Uses sphere-radius of object/shape.
		// This is based on Bullet's `getBoundingSphere` command.
		struct SphereType {};

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
					half_size(size/2.0f)
				{}

				//SizeType(const SizeType&) = default;
				//SizeType(SizeType&&) = default;

				inline float get_size() const { return (half_size * 2.0f); }
				inline float get_half_size() const { return half_size; }

				// Alternate implementation:
				//float get_size() const { return size; }
				//inline float get_half_size() const { return (size / 2.0f); }
		};

		using SizeConfig = std::variant<std::monostate, AABBType, SphereType, SizeType>;

		CollisionCastMethod cast_method = CollisionCastMethod::None;
		SizeConfig size = std::monostate{};
	};
}