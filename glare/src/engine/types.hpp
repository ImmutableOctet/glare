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

	// NOTE: Must be equivalent to `app::input::PlayerInputID`.
	using PlayerIndex = std::uint16_t;

	// TODO: Remove/rework.
	enum class EntityType : std::uint8_t; // std::uint16_t

	// TODO: Remove forward declaration.
	enum class LightType : std::uint8_t;

	inline constexpr Entity null = static_cast<Entity>(entt::null); // entt::null_t

	inline constexpr PlayerIndex PRIMARY_LOCAL_PLAYER = 1;
	inline constexpr PlayerIndex NO_PLAYER            = 0;
	inline constexpr PlayerIndex ANY_PLAYER           = 0;

	// The number of frames per-second targeted by default.
	inline constexpr FramerateType DEFAULT_FRAMERATE = 60;

	// A fraction of a second representing the length of a 'frame'.
	inline constexpr float DEFAULT_RATE = (1.0f / static_cast<float>(DEFAULT_FRAMERATE));
}