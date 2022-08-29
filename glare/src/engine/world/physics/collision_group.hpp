#pragma once

//#include <cstdint>
#include <types.hpp> // Global header.

namespace engine
{
	enum class CollisionGroup : std::uint32_t // unsigned int
	{
		All = (UINT32_MAX),

		None = 0,

		// Bit 1 is reserved.

		StaticGeometry   = (1 << 1),
		DynamicGeometry  = (1 << 2),
		Actor            = (1 << 3),
		Object           = (1 << 4),
		Bone             = (1 << 5),
		Zone             = (1 << 6),
		Particle         = (1 << 7),
		Projectile       = (1 << 8),

		Meta = (Zone), // | Bone
		AllGeometry = (StaticGeometry | DynamicGeometry),
		
		GeometrySolids = (All & ~(Meta)),
		ObjectSolids = (AllGeometry | Actor | Object),
		BoneSolids = ObjectSolids,
		ActorSolids = ObjectSolids,
		ProjectileSolids = (ObjectSolids | Projectile | Bone),


		PlayerInteractions      = All, // (All & ~(StaticGeometry)),

		ObjectInteractions      = (Actor | Object | Zone | AllGeometry),
		CollectableInteractions = (Actor | Zone),
		EnemyInteractions       = ObjectInteractions, // (Actor | Object | Zone),

		// Used for hitscan bullets, projectiles, etc.
		HitDetectionInteractions = (ObjectSolids | Zone | Bone),
	};

	FLAG_ENUM(std::uint32_t, CollisionGroup); // unsigned int

	//using CollisionMask = CollisionGroup;
}