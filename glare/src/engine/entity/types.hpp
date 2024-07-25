#pragma once

#include <engine/types.hpp>

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

	using EntityThreadCount = EntityThreadIndex;
	using EntityThreadID = StringHash;

	using PrecompiledScriptID = entt::id_type; // StringHash;

	inline constexpr EntityThreadIndex      ENTITY_THREAD_INDEX_INVALID      = static_cast<EntityThreadIndex>(-1ull);
	inline constexpr EntityInstructionIndex ENTITY_INSTRUCTION_INDEX_INVALID = static_cast<EntityInstructionIndex>(-1ull);
	inline constexpr EntityStateIndex       ENTITY_STATE_INDEX_INVALID       = static_cast<EntityStateIndex>(-1ull);
}