#pragma once

#include "update_level.hpp"

#include <array>

namespace engine
{
	struct EntityBatchPriorityData
	{
		using BatchPriority = engine::UpdateLevel;

		inline static constexpr std::size_t batch_entry_count = static_cast<std::size_t>(BatchPriority::Count);

		template <typename T>
		using PerBatch = std::array<T, batch_entry_count>;

		PerBatch<UpdateInterval> update_intervals {};
	};
}