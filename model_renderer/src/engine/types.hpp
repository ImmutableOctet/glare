#pragma once

#include <entt/entt.hpp>
#include <types.hpp>

namespace engine
{
	using Registry   = entt::registry;
	using Entity     = entt::entity;
	using EventHandler = entt::dispatcher;

	/*
	template <typename... delegate_params>
	using Delegate = entt::delegate<delegate_params...>;

	template <typename... sink_parameters>
	using Sink = entt::sink<sink_parameters...>;
	*/

	class World;

	constexpr auto& null = entt::null;
}