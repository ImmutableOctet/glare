#pragma once

//#include <cstdint>
#include <types.hpp> // Global header.

namespace engine
{
	enum class CollisionGroup : std::uint32_t // unsigned int
	{
		// Bitmask/filter for all collision groups.
		All = (UINT32_MAX),

		// No collision.
		None = 0,

		// Bit 1 is reserved.

		// Collision groups:
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
		GeometrySolids   = (All & ~(Meta)),
		ObjectSolids     = (AllGeometry | Actor | Object),
		BoneSolids       = ObjectSolids,
		ActorSolids      = ObjectSolids,
		ProjectileSolids = (ObjectSolids | Projectile | Bone),

		// 'Interaction' filters for different groups:
		PlayerInteractions      = All, // (All & ~(StaticGeometry)),
		ObjectInteractions      = (Actor | Object | Zone | AllGeometry),
		CollectableInteractions = (Actor | Zone), // `Actor` for obvious reasons, `Zone` for kill-planes, etc.
		EnemyInteractions       = ObjectInteractions, // (Actor | Object | Zone),

		// Used for hitscan bullets, projectiles, etc.
		// (`ObjectSolids` to simplify, `Zone` to reduce projectile clutter, `Bone`s for hit exact detection, etc.)
		HitDetectionInteractions = (ObjectSolids | Zone | Bone),
	};

	FLAG_ENUM(std::uint32_t, CollisionGroup); // unsigned int

	using CollisionMask = CollisionGroup;

	// Determines whether a `CollisionGroup` should be kinematic by default.
	// NOTE: As a rule of thumb, not all entities within these groups will have the same kinematic status.
	bool collision_group_is_kinematic(CollisionGroup group);
}