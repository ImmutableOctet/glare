#pragma once

#include <entt/entt.hpp>
#include <types.hpp>

namespace engine
{
	using Registry     = entt::registry;
	using Entity       = entt::entity;
	using EventHandler = entt::dispatcher;

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
	}; FLAG_ENUM(std::uint8_t, Axis);

	enum class LightType : std::uint8_t
	{
		Point,
		Directional,
		Spotlight,
	}; FLAG_ENUM(std::uint8_t, LightType);

	constexpr auto& null = entt::null;
}