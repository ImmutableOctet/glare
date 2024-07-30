#pragma once

#include <engine/time_decl.hpp>
#include <engine/rates.hpp>

#include <chrono>

namespace engine
{
	namespace network
	{
		namespace time
		{
			using namespace engine::time::shared;

			using Clock          = std::chrono::steady_clock;
			using TimePoint      = CustomTimePoint<Clock>; // Clock::time_point;
			using Duration       = Clock::duration;
			using DurationRaw    = Duration::rep;
			using Interval       = Duration;

			namespace impl
			{
				template <typename DurationType>
				constexpr Duration duration_cast(DurationType duration_value)
				{
					return engine::time::shared::impl::duration_cast<Duration>(duration_value);
				}
			}

			constexpr Duration to_duration(FloatSeconds seconds)  { return engine::time::shared::impl::to_duration<Duration>(seconds); }
			constexpr Duration to_duration(DoubleSeconds seconds) { return engine::time::shared::impl::to_duration<Duration>(seconds); }
			constexpr Duration to_duration(float seconds)         { return engine::time::shared::impl::to_duration<Duration>(seconds); }
			constexpr Duration to_duration(double seconds)        { return engine::time::shared::impl::to_duration<Duration>(seconds); }

			constexpr auto tick_rate(auto frames_per_second)
			{
				return engine::time::shared::impl::tick_rate<Duration>(frames_per_second);
			}

			constexpr auto tick_rate()
			{
				return tick_rate(TARGET_NETWORK_UPDATES_PER_SECOND);
			}
		}
	}

	using NetworkClock          = network::time::Clock;
	using NetworkDuration       = network::time::Duration;
	using NetworkInterval       = network::time::Interval;
	using NetworkUpdateInterval = network::time::Interval;
	using NetworkRate           = network::time::Interval;
	using NetworkUpdateRate     = network::time::Interval;
	using NetworkLatency        = network::time::Milliseconds;
}