#pragma once

#include <types.hpp>
#include <graphics/types.hpp>

#include <entt/entt.hpp>

#include <string>

// TODO: Look into removing this from the main 'types' header.
namespace graphics
{
	struct Animation;
}

namespace engine
{
	// Standard forward declarations:
	class World;
	class ResourceManager;

	// TODO: Look into removing this from the main 'types' header.
	struct AnimationData;

	using Registry     = entt::registry;
	using Entity       = entt::entity;
	using EntityIDType = entt::id_type;
	using EventHandler = entt::dispatcher;

	/*
	template <typename... delegate_params>
	using Delegate = entt::delegate<delegate_params...>;

	template <typename... sink_parameters>
	using Sink = entt::sink<sink_parameters...>;
	*/

	using Animation    = graphics::Animation;
	using AnimationID  = graphics::AnimationID;
	using BoneID       = graphics::BoneID;

	// NOTE: Must be equivalent to `app::input::PlayerInputID`.
	using PlayerIndex = std::uint16_t;

	enum class Axis : std::uint8_t
	{
		Pitch = (1 << 0),
		Yaw   = (1 << 1),
		Roll  = (1 << 2),
	};
	
	FLAG_ENUM(std::uint8_t, Axis);

	enum class LightType : std::uint8_t
	{
		Point,
		Directional,
		Spotlight,
	};
	
	FLAG_ENUM(std::uint8_t, LightType);

	enum class EntityType : std::uint8_t // std::uint16_t
	{
		Other = 0,

		Geometry,      // Stage geometry, etc.
		Scenery,       // Scenery, such as decorative objects - e.g. grass, etc. (No solid collision)
		Platform,      // Dynamic stage geometry; platforms, etc.
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

		Default = Other
	};

	constexpr auto& null = entt::null;

	constexpr PlayerIndex PRIMARY_LOCAL_PLAYER = 1;
}