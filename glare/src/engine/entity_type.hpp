#pragma once

// TODO: Remove in favor of forward declaration header.
#include <engine/types.hpp>

namespace engine
{
	// TODO: Remove/rework.
	enum class EntityType : EntityTypeRaw
	{
		Default = 0,

		Geometry,      // Level geometry, etc.
		Scenery,       // Scenery, such as decorative objects - e.g. grass, etc. (No solid collision)
		Platform,      // Dynamic level geometry; platforms, etc.
		Crusher,       // Crusher objects.
		Light,         // Any type of lighting object.
		Pivot,         // Generic pivots; used for physics, object behaviors, etc.
		Bone,          // Similar to 'Pivot', but for animations.
		Object,        // Standard objects, e.g. collectibles.
		Player,        // Player objects of any character type.
		Camera,        // Any type of camera; orthographic and perspective.
		Enemy,         // Enemy actors of any type. (Hostile)
		FriendlyActor, // Friendly actors of any type. (NPCs, etc.)
		Collectable,   // Collectable objects.
		Particle,      // Fire effects, air bubbles, motion lines, etc.
		Projectile,    // Bullets, energy blasts, etc.
		EventTrigger,  // Meta objects that trigger events.

		//Regulator,   // Kill planes, etc.
		WaterZone,     // Environmental, // e.g. Water objects.
		KillZone,      // Destroys entities that interact with it. (Death plane)
		DamageZone,    // Causes damage to all damageable entities. (e.g. entities with a health component)
		
		Generator,     // An entity which generates other entities. (e.g. spawing enemies)

		Other = Default
	};
}