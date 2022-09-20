#include "collision_group.hpp"

namespace engine
{
	// TODO: Look into implementing this via bitflag checks, rather than exact values.
	CollisionBodyType get_collision_body_type(CollisionGroup group)
	{
		switch (group)
		{
			case CollisionGroup::StaticGeometry:
				return CollisionBodyType::Static;
			case CollisionGroup::Zone:
				return CollisionBodyType::Static;
			case CollisionGroup::DynamicGeometry:
				return CollisionBodyType::Dynamic;
			case CollisionGroup::Projectile:
				// May change this later.
				return CollisionBodyType::Dynamic;
		}

		return CollisionBodyType::Kinematic;
	}

	bool collision_group_is_kinematic(CollisionGroup group)
	{
		/*
		using enum CollisionGroup;

		// Determine which groups are NOT considered kinematic (by default):
		switch (group)
		{
			case StaticGeometry:
				return false; // Static geometry, by definition, should not be affected by collision impulses. ('dynamic' is fair game)
			case Zone:
				return false; // Zones generally should not be impacted by collisions.
			case Particle:
				return false; // May change this later.
			case Projectile:
				return false; // May change this later. (doesn't make sense to allow kinematics for these)
		}

		// All groups not found in the above switch-statement are kinematic.
		return true;
		*/

		return (get_collision_body_type(group) == CollisionBodyType::Kinematic);
	}
}