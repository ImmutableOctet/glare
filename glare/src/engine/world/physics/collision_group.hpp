#pragma once

#include "collision_body_type.hpp"

#include <util/enum_operators.hpp>

#include <cstdint>

namespace engine
{
	enum class CollisionGroup : std::uint32_t // std::uint16_t // unsigned int
	{
		// No collision.
		None = 0,

		// Bitmask/filter for all collision groups.
		All = (UINT32_MAX), // (UINT16_MAX >> 1),

		// Collision groups:

		Reserved         = (1 << 0),

		StaticGeometry   = (1 << 1),
		DynamicGeometry  = (1 << 2),
		Actor            = (1 << 3),
		Object           = (1 << 4),
		Bone             = (1 << 5),
		Zone             = (1 << 6),
		Particle         = (1 << 7),
		Projectile       = (1 << 8),

		// General filters/compound groups:
		Meta = (Zone), // | Bone
		AllGeometry = (StaticGeometry | DynamicGeometry),
		
		// 'Solid' filters for different groups:
		GeometrySolids   = (All & ~(Meta)), // UINT32_MAX
		ObjectSolids     = (AllGeometry | Actor | Object),
		BoneSolids       = ObjectSolids,
		ActorSolids      = ObjectSolids,
		ProjectileSolids = (ObjectSolids | Projectile | Bone),

		// 'Interaction' filters for different groups:
		PlayerInteractions      = (All & ~(StaticGeometry)), // UINT32_MAX
		ObjectInteractions      = (Actor | Object | Zone | AllGeometry),
		CollectableInteractions = (Actor | Zone), // `Actor` for obvious reasons, `Zone` for kill-planes, etc.
		EnemyInteractions       = ObjectInteractions, // (Actor | Object | Zone),

		// Used for hitscan bullets, projectiles, etc.
		// (`ObjectSolids` to simplify, `Zone` to reduce projectile clutter, `Bone`s for hit exact detection, etc.)
		HitDetectionInteractions = (ObjectSolids | Zone | Bone),
	};

	FLAG_ENUM(std::uint32_t, CollisionGroup); // unsigned int

	using CollisionMask = CollisionGroup;

	// Retrieves the default collision-body-type associated with a specific `CollisionGroup`.
	// (`group` must be an exact value; bitmasks are currently unsupported)
	CollisionBodyType get_collision_body_type(CollisionGroup group);

	// Determines whether a `CollisionGroup` should be kinematic by default.
	// NOTE: As a rule of thumb, not all entities within these groups will have the same kinematic status.
	// (`group` must be an exact value; bitmasks are currently unsupported)
	bool collision_group_is_kinematic(CollisionGroup group);
}