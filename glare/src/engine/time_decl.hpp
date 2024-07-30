#pragma once

#include "rates.hpp"

#include <chrono>

namespace engine
{
	namespace time
	{
		namespace shared
		{
			using Days          = std::chrono::days;
			using Hours         = std::chrono::hours;
			using Minutes       = std::chrono::minutes;
			using Seconds       = std::chrono::seconds;
			using Milliseconds  = std::chrono::milliseconds;
			using Microseconds  = std::chrono::microseconds;
			using Nanoseconds   = std::chrono::nanoseconds;

			using FloatSeconds  = std::chrono::duration<float>;
			using DoubleSeconds = std::chrono::duration<double>;

			template <typename RepType>
			using CustomDurationType = std::chrono::duration<RepType>;

			template <typename ClockType>
			using CustomTimePoint = std::chrono::time_point<ClockType>;

			using std::chrono::duration_cast;

			namespace impl
			{
				template <typename OutputDurationType, typename InputDurationType>
				constexpr OutputDurationType duration_cast(InputDurationType&& duration_value)
				{
					return std::chrono::duration_cast<OutputDurationType>(std::forward<InputDurationType>(duration_value));
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

				// Divides one second (converted to nanoseconds) by the `denominator` specified.
				constexpr auto second_divide(auto denominator)
				{
					return
					(
						duration_cast<Nanoseconds>(Seconds { 1 })
						/
						denominator
					);
				}

				template <typename OutputDurationType, typename FramesPerSecondType>
				constexpr OutputDurationType tick_rate(FramesPerSecondType&& frames_per_second)
				{
					return duration_cast<OutputDurationType>
					(
						second_divide(std::forward<FramesPerSecondType>(frames_per_second))
					);
				}
			}

			// Converts the specified rate (fraction of a second) into nanoseconds,
			// then scales it inversely relative to one full second (converted to nanoseconds).
			// 
			// The result is the number of times `rate` can happen in a second.
			constexpr auto per_second(auto&& rate)
			{
				return engine::time::shared::impl::second_divide(engine::time::shared::duration_cast<Nanoseconds>(rate));
			}
		}
	}

	namespace time
	{
		using namespace engine::time::shared;

		using Clock        = std::chrono::steady_clock; // std::chrono::system_clock; // std::chrono::high_resolution_clock;
		using TimePoint    = Clock::time_point; // CustomTimePoint<Clock>;
		using Duration     = Clock::duration;
		using DurationRaw  = Duration::rep;
		using Interval     = Duration;

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
			//return to_duration(engine::TARGET_FRAME_DELTA_EX);

			return engine::time::shared::impl::tick_rate<Duration>(frames_per_second);
		}

		constexpr auto tick_rate()
		{
			return tick_rate(TARGET_FRAMES_PER_SECOND);
		}
	}

	using SystemClock    = time::Clock;
	using TimeDuration   = time::Duration;
	using TimeInterval   = time::Interval;
	using TimePoint      = time::TimePoint;
	using UpdateInterval = time::Interval;

	using Days           = time::shared::Days;
	using Hours          = time::shared::Hours;
	using Minutes        = time::shared::Minutes;
	using Seconds        = time::shared::Seconds; 
	using Milliseconds   = time::shared::Milliseconds;
	using Microseconds   = time::shared::Microseconds;
	using Nanoseconds    = time::shared::Nanoseconds;
}