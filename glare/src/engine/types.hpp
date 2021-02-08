#pragma once

#include <entt/entt.hpp>
#include <math/math.hpp>
#include <types.hpp>

#include <vector>

namespace engine
{
	using Registry     = entt::registry;
	using Entity       = entt::entity;
	using EventHandler = entt::dispatcher;

	using Spline = std::vector<math::Vector>;

	/*
	template <typename... delegate_params>
	using Delegate = entt::delegate<delegate_params...>;

	template <typename... sink_parameters>
	using Sink = entt::sink<sink_parameters...>;
	*/

	class World;

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
		Light,         // Any type of lighting object.
		Pivot,         // Generic pivots; used for physics, object behaviors, etc.
		Bone,          // Similar to 'Pivot', but for animations.
		Object,        // Standard objects, e.g. collectibles.
		Player,        // Player objects of any character type.
		Camera,        // Any type of camera; orthographic and perspective.
		Particle,      // Fire effects, air bubbles, motion lines, etc.
		Environmental, // e.g. Water objects.
		EventTrigger,  // Meta objects that trigger events.
		Regulator,     // Kill planes, etc.
		Generator,     // An entity which generates more entities.

		Default = Other
	};

	constexpr auto& null = entt::null;
}