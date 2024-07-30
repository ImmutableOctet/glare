#pragma once

#include <cstdint>

namespace engine
{
	enum class EntityBatchID : std::uint8_t {};

	using UpdateLevelRaw = std::uint8_t;

	enum class UpdateLevel : UpdateLevelRaw;

	using BatchPriorityIndex = UpdateLevelRaw;
	using BatchPriorityCount = BatchPriorityIndex;
}