#pragma once

#include "types.hpp"

#include <chrono>

namespace engine
{
	namespace time
	{
		namespace shared
		{
			using Days           = std::chrono::days;
			using Hours          = std::chrono::hours;
			using Minutes        = std::chrono::minutes;
			using Seconds        = std::chrono::seconds; 
			using Milliseconds   = std::chrono::milliseconds;
			using Microseconds   = std::chrono::microseconds;

			using FloatSeconds   = std::chrono::duration<float>;
			using DoubleSeconds  = std::chrono::duration<double>;

			template <typename RepType>
			using CustomDurationType = std::chrono::duration<RepType>;

			template <typename ClockType>
			using CustomTimePoint = std::chrono::time_point<ClockType>;

			namespace impl
			{
				template <typename OutputDurationType, typename InputDurationType>
				OutputDurationType duration_cast(InputDurationType&& duration_value)
				{
					return std::chrono::duration_cast<OutputDurationType>(duration_value);
				}

				template <typename OutputDurationType, typename SecondsType>
				OutputDurationType to_duration_seconds_impl(SecondsType&& seconds)
				{
					return duration_cast<OutputDurationType>(std::chrono::round<std::chrono::nanoseconds>(std::forward<SecondsType>(seconds)));
				}

				template <typename OutputDurationType>
				OutputDurationType to_duration(FloatSeconds seconds)
				{
					return to_duration_seconds_impl<OutputDurationType>(seconds);
				}

				template <typename OutputDurationType>
				OutputDurationType to_duration(float seconds)
				{
					return to_duration<OutputDurationType>(FloatSeconds(seconds));
				}

				template <typename OutputDurationType>
				OutputDurationType to_duration(DoubleSeconds seconds)
				{
					return to_duration_seconds_impl<OutputDurationType>(seconds);
				}

				template <typename OutputDurationType>
				OutputDurationType to_duration(double seconds)
				{
					return to_duration<OutputDurationType>(DoubleSeconds(seconds));
				}
			}
		}

		using namespace engine::time::shared;

		using Clock          = std::chrono::system_clock; // std::chrono::high_resolution_clock;
		using TimePoint      = CustomTimePoint<Clock>; // Clock::time_point;
		using Duration       = Clock::duration;
		using DurationRaw    = Duration::rep;

		using Interval       = Duration;
		using UpdateInterval = Interval;

		inline Duration to_duration(FloatSeconds seconds)  { return engine::time::shared::impl::to_duration<Duration>(seconds); }
		inline Duration to_duration(DoubleSeconds seconds) { return engine::time::shared::impl::to_duration<Duration>(seconds); }
		inline Duration to_duration(float seconds)         { return engine::time::shared::impl::to_duration<Duration>(seconds); }
		inline Duration to_duration(double seconds)        { return engine::time::shared::impl::to_duration<Duration>(seconds); }

		inline Duration tick_rate()
		{
			return to_duration(engine::DEFAULT_RATE);
		}

		namespace impl
		{
			template <typename DurationType>
			Duration duration_cast(DurationType duration_value)
			{
				return engine::time::shared::impl::duration_cast<Duration>(duration_value);
			}
		}
	}

	using SystemClock    = time::Clock;
	using TimeDuration   = time::Duration;
	using TimeInterval   = time::Interval;
	using UpdateInterval = time::UpdateInterval;
}