#pragma once

#include <engine/types.hpp>

#include <util/small_vector.hpp>
//#include <vector>

#include <optional>

namespace engine
{
	// Opaque identifier used to differentiate entity states.
	using EntityStateHash = entt::id_type;

	// Identifier used to represent an index of an entity state.
	using EntityStateIndex = std::uint16_t; // std::uint8_t;

	using EntityStateCount = EntityStateIndex; // std::uint16_t; // std::uint8_t;

	using EntityStateID = StringHash;

	using EntityInstructionIndex = std::uint16_t; // std::uint32_t; // std::size_t;
	using EntityInstructionCount = EntityInstructionIndex;

	using EntityThreadIndex = SharedStorageIndex; // std::uint16_t; // std::uint32_t;
	//enum class EntityThreadIndex : SharedStorageIndex {};

	using EntityThreadCount = EntityThreadIndex;
	using EntityThreadID = StringHash;

	//using EntityVector = util::small_vector<Entity, 16>; // std::vector<Entity>; // std::unordered_set<Entity>;

	struct EntityStateInfo
	{
		EntityStateIndex index;
		std::optional<EntityStateHash> id; // EntityStateHash
	};
}