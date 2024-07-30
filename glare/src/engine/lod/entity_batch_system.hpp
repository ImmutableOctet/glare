#pragma once

#include "types.hpp"

#include "update_level.hpp"

#include <engine/basic_system.hpp>

#include <engine/delta_time.hpp>
#include <engine/time_decl.hpp>

#include <mutex>

namespace engine
{
	struct EntityBatchPriorityData;

	struct OnServiceUpdate;

	class EntityBatchSystem : public BasicSystem
	{
		public:
			using BatchPriorityData = EntityBatchPriorityData;

			using Clock             = time::Clock;
			using Duration          = time::Duration;

			using BatchTimer        = DeltaTime;

			using BatchMutex        = std::recursive_mutex;
			using BatchLock         = std::scoped_lock<BatchMutex>;

			EntityBatchSystem
			(
				Service& service,
				const BatchPriorityData& priority_data,
				bool subscribe_immediately=false
			);

			EntityBatchSystem
			(
				Service& service,
				bool subscribe_immediately=false
			);

			void set_update_intervals(const BatchPriorityData& priority_data, bool reset_timers=true);

			BatchPriorityIndex get_index(UpdateLevel update_level) const;

			float get_delta(UpdateLevel update_level) const;

			BatchLock lock(UpdateLevel update_level);

		protected:
			static constexpr BatchPriorityIndex BATCH_INDEX_MIN   = static_cast<BatchPriorityIndex>(UpdateLevel::Min);
			static constexpr BatchPriorityIndex BATCH_INDEX_MAX   = static_cast<BatchPriorityIndex>(UpdateLevel::Max);
			static constexpr BatchPriorityCount BATCH_INDEX_COUNT = (BATCH_INDEX_MAX - BATCH_INDEX_MIN);

			template <typename T>
			using PerBatch = std::array<T, BATCH_INDEX_COUNT>;

			template <typename T>
			using PerUpdateLevel = PerBatch<T>;

			inline constexpr BatchPriorityCount batch_count() const
			{
				return BATCH_INDEX_COUNT;
			}

			bool on_subscribe(Service& service) override;
			bool on_unsubscribe(Service& service) override;

			void on_update(const OnServiceUpdate& data);

			void on_transform_changed(Registry& registry, Entity entity);

			bool on_batch_update(BatchPriorityIndex batch, BatchTimer& batch_timer, Duration time_elapsed);

			void reset_batch_timers();

			PerBatch<BatchTimer> batch_timers = {};
			PerBatch<BatchMutex> batch_locks  = {};
	};
}