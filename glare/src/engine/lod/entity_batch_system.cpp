#include "entity_batch_system.hpp"

#include "entity_batch_priority_data.hpp"

#include "events/on_batch_update.hpp"

#include <engine/types.hpp>
#include <engine/time.hpp>
#include <engine/service.hpp>
#include <engine/service_events.hpp>

#include <engine/components/transform_component.hpp>

#include <chrono>

namespace engine
{
	namespace impl
	{
		static EntityBatchSystem::BatchPriorityData generate_default_batch_priority_data()
		{
			using namespace std::literals::chrono_literals;

			constexpr auto UPDATE_INTERVAL_NEVER = engine::time::Interval::max();
			constexpr auto FRAME_TIME            = engine::time::tick_rate();

			using engine::time::to_duration;

			return EntityBatchSystem::BatchPriorityData
			{
				.update_intervals =
				{
					(UPDATE_INTERVAL_NEVER),              // StaticContent
					to_duration(1s),                      // SemiStaticContent
					to_duration(500ms),                   // Distant
					
					(FRAME_TIME),                         // Fixed
					to_duration(2.0f * FRAME_TIME),       // Half

					to_duration(8.0f * FRAME_TIME),       // VeryLow
					to_duration(4.0f * FRAME_TIME),       // Low
					to_duration(2.0f * FRAME_TIME),       // Medium
					to_duration(1.0f * FRAME_TIME),       // High

					to_duration(0us)                      // VeryHigh (Realtime)
				}
			};
		}
	}

	EntityBatchSystem::EntityBatchSystem
	(
		Service& service,
		const BatchPriorityData& priority_data,
		bool subscribe_immediately
	) :
		BasicSystem(service)
	{
		if (subscribe_immediately)
		{
			subscribe();
		}

		set_update_intervals(priority_data);
	}

	EntityBatchSystem::EntityBatchSystem(Service& service, bool subscribe_immediately) :
		EntityBatchSystem(service, impl::generate_default_batch_priority_data(), subscribe_immediately)
	{}

	bool EntityBatchSystem::on_subscribe(Service& service)
	{
		auto& registry = service.get_registry();

		// Registry events:
		registry.on_update<TransformComponent>().connect<&EntityBatchSystem::on_transform_changed>(*this);

		// Standard events:
		service.register_event<OnServiceUpdate, &EntityBatchSystem::on_update>(*this);

		return true;
	}

	bool EntityBatchSystem::on_unsubscribe(Service& service)
	{
		service.unregister(*this);

		return true;
	}

	void EntityBatchSystem::on_update(const OnServiceUpdate& data)
	{
		for (auto batch_index = BATCH_INDEX_MIN; batch_index < batch_count(); ++batch_index)
		{
			auto& batch_timer = batch_timers[batch_index];

			auto batch_timestamp = batch_timer.current_frame_time();

			const auto update_timestamp = Clock::now();
			const auto time_since_last_update = (update_timestamp - batch_timestamp);

			const auto update_interval = batch_timer.get_interval();

			if (time_since_last_update >= update_interval)
			{
				const auto update_timestamp_ms = engine::time::duration_cast<Milliseconds>(time_since_last_update);

				if (on_batch_update(batch_index, batch_timer, update_timestamp_ms))
				{
					////batch_timer.update(update_timestamp);
				}
			}
		}
	}

	bool EntityBatchSystem::on_batch_update(BatchPriorityIndex batch, BatchTimer& batch_timer, Duration time_elapsed)
	{
		auto& service = get_service();

		service.event<OnBatchUpdate>(time_elapsed, batch_timer.get_delta(), static_cast<UpdateLevel>(batch));

		return true;
	}

	void EntityBatchSystem::on_transform_changed(Registry& registry, Entity entity)
	{
		
	}

	void EntityBatchSystem::set_update_intervals(const BatchPriorityData& priority_data, bool reset_timers)
	{
		auto batch_index = BATCH_INDEX_MIN;

		const auto& update_intervals = priority_data.update_intervals;

		for (auto& batch_timer : batch_timers)
		{
			const auto& update_interval = update_intervals[batch_index];

			batch_timer.set_interval(update_interval);

			++batch_index;
		}

		if (reset_timers)
		{
			reset_batch_timers();
		}
	}

	void EntityBatchSystem::reset_batch_timers()
	{
		for (auto& batch_timer : batch_timers)
		{
			batch_timer.reset(Clock::now());
		}
	}

	BatchPriorityIndex EntityBatchSystem::get_index(UpdateLevel update_level) const
	{
		return static_cast<BatchPriorityIndex>(update_level);
	}

	float EntityBatchSystem::get_delta(UpdateLevel update_level) const
	{
		const auto batch_index = get_index(update_level);

		return batch_timers[batch_index].get_delta();
	}

	EntityBatchSystem::BatchLock EntityBatchSystem::lock(UpdateLevel update_level)
	{
		const auto batch_index = get_index(update_level);

		auto& batch_mutex = batch_locks[batch_index];

		return BatchLock { batch_mutex };
	}
}