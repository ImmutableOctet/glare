#pragma once

#include <engine/time_decl.hpp>

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
			using UpdateInterval = Interval;

			inline Duration to_duration(FloatSeconds seconds)  { return engine::time::shared::impl::to_duration<Duration>(seconds); }
			inline Duration to_duration(DoubleSeconds seconds) { return engine::time::shared::impl::to_duration<Duration>(seconds); }
			inline Duration to_duration(float seconds)         { return engine::time::shared::impl::to_duration<Duration>(seconds); }
			inline Duration to_duration(double seconds)        { return engine::time::shared::impl::to_duration<Duration>(seconds); }

			namespace impl
			{
				template <typename DurationType>
				Duration duration_cast(DurationType duration_value)
				{
					return engine::time::shared::impl::duration_cast<Duration>(duration_value);
				}
			}
		}
	}

	using NetworkClock          = network::time::Clock;
	using NetworkDuration       = network::time::Duration;
	using NetworkInterval       = network::time::Interval;
	using NetworkUpdateInterval = network::time::UpdateInterval;
}