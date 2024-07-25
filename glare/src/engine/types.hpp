#pragma once

//#include <entt/entt.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/entity.hpp>

#include <cstdint>

namespace engine
{
	using SharedStorageIndex = std::uint16_t; // std::uint32_t; // std::size_t; // util::DefaultSharedStorageIndex;

	using Registry     = entt::registry;
	using Entity       = entt::entity;
	using EntityIDType = entt::id_type;

	using StringHash = entt::id_type; // entt::hashed_string;

	// TODO: Remove/rework:
	using EntityTypeRaw = std::uint8_t; // std::uint16_t;

	enum class EntityType : EntityTypeRaw;

	// TODO: Remove from this file.
	using LightTypeRaw = std::uint8_t;

	// TODO: Remove forward declaration.
	enum class LightType : LightTypeRaw;

	inline constexpr Entity null = static_cast<Entity>(entt::null); // entt::null_t
}