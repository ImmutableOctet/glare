#pragma once

#include <cstdint>

#include <graphics/types.hpp>

#include <util/enum_operators.hpp>

//#include <entt/entt.hpp>
//#include <entt/entity/fwd.hpp>
#include <entt/entity/entity.hpp>

//#include <string>

// TODO: Look into removing this from the main 'types' header.
namespace graphics
{
	struct AnimationData;
}

namespace engine
{
	// Standard forward declarations:
	class Service;
	class World;
	class ResourceManager;

	// TODO: Look into removing this from the main 'types' header.
	struct AnimationData;

	using FramerateType = std::uint32_t;

	using SharedStorageIndex = std::uint16_t; // std::uint32_t; // std::size_t; // util::DefaultSharedStorageIndex;

	using Registry     = entt::registry;
	using Entity       = entt::entity;
	using EntityIDType = entt::id_type;

	using StringHash = entt::id_type; // entt::hashed_string;

	/*
	template <typename... delegate_params>
	using Delegate = entt::delegate<delegate_params...>;

	template <typename... sink_parameters>
	using Sink = entt::sink<sink_parameters...>;
	*/

	using AnimationID      = entt::id_type; // graphics::AnimationID;
	using AnimationLayerID = entt::id_type;
	using BoneID           = entt::id_type; // graphics::BoneID;
	using BoneIndex        = graphics::BoneIndex;

	// A type representing a bitmask used to filter which bones an animation is applied to.
	using AnimationLayerMask = std::uint8_t;

	// Index type used for animation layers.
	using AnimationLayerIndex = std::size_t;

	using AnimationLayerCount = AnimationLayerIndex;

	// NOTE: Must be equivalent to `app::input::PlayerInputID`.
	using PlayerIndex = std::uint16_t;

	// TODO: Rename to something less generic. (i.e. RotationAxis)
	enum class Axis : std::uint8_t
	{
		Pitch = (1 << 0),
		Yaw   = (1 << 1),
		Roll  = (1 << 2),
	};
	
	FLAG_ENUM(std::uint8_t, Axis);

	enum class LightType : std::uint8_t
	{
		// Allows for multiple light sub-components
		// to be bound to the same entity.
		Any,

		// Specific types:
		Point,
		Directional,
		Spot,

		Spotlight = Spot,
	};
	
	FLAG_ENUM(std::uint8_t, LightType);

	enum class EntityType : std::uint8_t // std::uint16_t
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

	inline constexpr const entt::null_t& null = entt::null;

	inline constexpr PlayerIndex PRIMARY_LOCAL_PLAYER = 1;
	inline constexpr PlayerIndex NO_PLAYER            = 0;
	inline constexpr PlayerIndex ANY_PLAYER           = 0;

	inline constexpr AnimationLayerMask ANIMATION_LAYER_MASK_NO_LAYER = {};

	// Maximum number of concurrent animation layers. (Number of bits)
	inline constexpr std::size_t MAX_ANIMATION_LAYERS = (sizeof(AnimationLayerMask) * 8);

	// The number of frames per-second targeted by default.
	inline constexpr FramerateType DEFAULT_FRAMERATE = 60;

	// A fraction of a second representing the length of a 'frame'.
	inline constexpr float DEFAULT_RATE = (1.0f / static_cast<float>(DEFAULT_FRAMERATE));
}