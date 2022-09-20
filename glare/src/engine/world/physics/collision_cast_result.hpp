#pragma once

#include <engine/types.hpp>
#include <math/math.hpp>

class btCollisionObject;

namespace engine
{
	// TODO: Unify these result types with their corresponding event types.
	
	struct CollisionCastResult
	{
		Entity cast_entity;
		Entity hit_entity;

		math::Vector hit_position;
		math::Vector hit_normal;

		float closest_hit_fraction;

		struct
		{
			// Bullet native:
			const btCollisionObject* cast_object;
			const btCollisionObject* hit_object;
		} native;
	};
	
	struct ConvexCastResult : public CollisionCastResult
	{
	};

	struct RayCastResult : public CollisionCastResult
	{
	};
}