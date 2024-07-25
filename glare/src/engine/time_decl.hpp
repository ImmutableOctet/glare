#pragma once

#include "rates.hpp"

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
			using Nanoseconds    = std::chrono::nanoseconds;

			using FloatSeconds   = std::chrono::duration<float>;
			using DoubleSeconds  = std::chrono::duration<double>;

			template <typename RepType>
			using CustomDurationType = std::chrono::duration<RepType>;

			template <typename ClockType>
			using CustomTimePoint = std::chrono::time_point<ClockType>;

			namespace impl
			{
				template <typename OutputDurationType, typename InputDurationType>
				constexpr OutputDurationType duration_cast(InputDurationType&& duration_value)
				{
					return std::chrono::duration_cast<OutputDurationType>(duration_value);
				}

				template <typename OutputDurationType, typename SecondsType>
				constexpr OutputDurationType to_duration_seconds_impl(SecondsType&& seconds)
				{
					return duration_cast<OutputDurationType>(std::chrono::round<std::chrono::nanoseconds>(std::forward<SecondsType>(seconds)));
				}

				template <typename OutputDurationType>
				constexpr OutputDurationType to_duration(FloatSeconds seconds)
				{
					return to_duration_seconds_impl<OutputDurationType>(seconds);
				}

				template <typename OutputDurationType>
				constexpr OutputDurationType to_duration(float seconds)
				{
					return to_duration<OutputDurationType>(FloatSeconds(seconds));
				}

				template <typename OutputDurationType>
				constexpr OutputDurationType to_duration(DoubleSeconds seconds)
				{
					return to_duration_seconds_impl<OutputDurationType>(seconds);
				}

				template <typename OutputDurationType>
				constexpr OutputDurationType to_duration(double seconds)
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
		using Rate           = Duration;
		using UpdateRate     = Rate;

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

		constexpr Interval tick_rate()
		{
			//return to_duration(engine::TARGET_FRAME_DELTA_EX);

			return engine::time::impl::duration_cast
			(
				std::chrono::duration_cast<Nanoseconds>(Seconds { 1 })
				/
				TARGET_FRAMES_PER_SECOND
			);
		}
	}

	using SystemClock    = time::Clock;
	using TimeDuration   = time::Duration;
	using TimeInterval   = time::Interval;
	using UpdateInterval = time::UpdateInterval;
}