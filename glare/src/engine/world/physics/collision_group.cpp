#include "collision_group.hpp"

namespace engine
{
	bool collision_group_is_kinematic(CollisionGroup group)
	{
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
	}
}