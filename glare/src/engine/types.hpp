#pragma once

//#include <entt/entt.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/entity.hpp>

#include <cstdint>

namespace engine
{
	// Standard forward declarations:
	class Service;
	class World;
	class ResourceManager;

	// TODO: Look into removing this from the main 'types' header.
	struct AnimationData;

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

	// TODO: Remove/rework.
	enum class EntityType : std::uint8_t; // std::uint16_t

	// TODO: Remove forward declaration.
	enum class LightType : std::uint8_t;

	inline constexpr Entity null = static_cast<Entity>(entt::null); // entt::null_t
}