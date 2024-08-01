#pragma once

#include "time_decl.hpp"
#include "rates.hpp"

#include <array>
#include <algorithm>
#include <type_traits>
#include <numeric>
#include <limits>

#include <cassert>
#include <cstdint>
#include <cmath>

namespace engine
{
	inline static constexpr std::size_t DEFAULT_DELTALOG_SIZE = 20;

	namespace impl
	{
		template <typename ScalarType, typename TicksPerSecondType, typename TimeType, typename IntervalType, typename IndexType=std::uint8_t, std::size_t log_size=DEFAULT_DELTALOG_SIZE>
		class DeltaTimeImpl
		{
			public:
				static_assert
				(
					(
						(std::is_same_v<IndexType, std::size_t>)
						||
						(log_size < static_cast<std::size_t>(std::numeric_limits<IndexType>::max() + 1))
					),

					"The `IndexType` specified is not large enough to access every element of the delta log. (`log_size` is too large or `IndexType` is too small)"
				);

				using Scalar = ScalarType;
				using TicksPerSecond = TicksPerSecondType;
				using Time = TimeType;
				using Index = IndexType;
				using Interval = IntervalType;

				static constexpr Interval calculate_interval(TicksPerSecond ideal_rate)
				{
					constexpr auto unspecified_rate = TicksPerSecond {};

					if (ideal_rate == unspecified_rate)
					{
						return {};
					}

					return engine::time::shared::impl::tick_rate<Interval>(ideal_rate);
				}

				static constexpr TicksPerSecond calculate_rate(Interval ideal_interval)
				{
					constexpr auto unspecified_interval = Interval {};

					if (ideal_interval == unspecified_interval)
					{
						return {};
					}

					return static_cast<TicksPerSecond>(engine::time::shared::per_second(ideal_interval));
				}

				static constexpr Scalar get_inv_delta(Scalar delta)
				{
					return (static_cast<Scalar>(1.0) / delta);
				}

			protected:
				//static constexpr Scalar LOG_SCALAR = ((static_cast<Scalar>(1.5) / static_cast<Scalar>(log_size)) * static_cast<Scalar>(0.75));
				static constexpr Scalar LOG_SCALAR = (static_cast<Scalar>(1.0) / static_cast<Scalar>(log_size));
			
				static constexpr Scalar DEFAULT_DELTA = static_cast<Scalar>(1.0);
				static constexpr Scalar DEFAULT_INV_DELTA = get_inv_delta(DEFAULT_DELTA);
				static constexpr Scalar DEFAULT_MINIMUM_DELTA = static_cast<Scalar>(0.0);
				static constexpr Scalar DEFAULT_MAXIMUM_DELTA = static_cast<Scalar>(4.0);

				static constexpr Index DEFAULT_NODE = static_cast<Index>(0);
			
				std::array<Scalar, log_size> delta_log;

				Scalar delta         = DEFAULT_DELTA;
				Scalar inv_delta     = DEFAULT_INV_DELTA;
				Scalar minimum_delta = DEFAULT_MINIMUM_DELTA;
				Scalar maximum_delta = DEFAULT_MAXIMUM_DELTA;

				Index delta_node = DEFAULT_NODE;

				Time time_previous_frame;
				Time time_current_frame;

				Interval ideal_interval;

				TicksPerSecond ideal_rate;

			public:
				constexpr DeltaTimeImpl(TicksPerSecond ideal_rate={}, Time current_time={}, Scalar minimum_delta=DEFAULT_MINIMUM_DELTA, Scalar maximum_delta=DEFAULT_MAXIMUM_DELTA) :
					minimum_delta(minimum_delta),
					maximum_delta(maximum_delta)
				{
					set_rate(ideal_rate);
					reset(current_time);
				}

				constexpr operator Scalar() const  { return delta;     }
				constexpr Scalar operator~() const { return inv_delta; }

				constexpr std::size_t size() const { return log_size; }

				constexpr Scalar         get_delta()     const { return delta;          }
				constexpr Scalar         get_inv_delta() const { return inv_delta;      }
				constexpr Interval       get_interval()  const { return ideal_interval; }
				constexpr TicksPerSecond get_rate()      const { return ideal_rate;     }

				constexpr Time current_frame_time() const { return time_current_frame; }

				template <typename T>
				constexpr T per_frame(T frame_diff)
				{
					return (frame_diff * delta);
				}

				constexpr void set_rate(TicksPerSecond ideal_rate)
				{
					this->ideal_rate = ideal_rate;
					this->ideal_interval = calculate_interval(ideal_rate);
				}

				constexpr void set_interval(Interval ideal_interval)
				{
					this->ideal_interval = ideal_interval;
					this->ideal_rate = calculate_rate(ideal_interval);
				}

				constexpr void reset_log()
				{
					delta_log.fill(DEFAULT_DELTA);
				}

				constexpr void reset(Time time, bool catch_up=false)
				{
					time_previous_frame = ((catch_up) ? time_current_frame : time);
					time_current_frame = time;

					delta = DEFAULT_DELTA;
					delta_node = DEFAULT_NODE;

					reset_log();
				}

				constexpr void reset(TicksPerSecond ideal_rate, Time time, bool catch_up=false)
				{
					set_rate(ideal_rate);

					reset(time, catch_up);
				}

				constexpr Scalar update(Time time)
				{
					// Ensure a valid frame time has been established prior to updating.
					assert((time_current_frame.time_since_epoch().count() > 0));

					// Ensure the time specified is newer than our previous time.
					assert((time >= time_current_frame));

					time_previous_frame = time_current_frame;
					time_current_frame = time;

					const auto frame_time = engine::time::duration_cast<Interval>(time_current_frame - time_previous_frame);

					const auto ideal_interval_raw = ideal_interval.count();

					const auto frame_delta = (ideal_interval_raw > 0)
						? (static_cast<Scalar>(frame_time.count()) / static_cast<Scalar>(ideal_interval_raw))
						: DEFAULT_DELTA
					;

					delta_log[delta_node] = std::clamp(frame_delta, minimum_delta, maximum_delta);

					delta_node = ((delta_node + 1) % size());

					const auto delta_sum = std::accumulate(delta_log.begin(), delta_log.end(), Scalar {});
					const auto delta_mean = (delta_sum * LOG_SCALAR);
					
					delta = delta_mean;

					inv_delta = get_inv_delta(delta);

					return delta;
				}

				constexpr friend DeltaTimeImpl& operator<<(DeltaTimeImpl& delta_time, Time time)
				{
					delta_time.update(time);

					return delta_time;
				}

				constexpr friend const DeltaTimeImpl& operator>>(const DeltaTimeImpl& delta_time, Scalar& delta_out)
				{
					delta_out = delta_time.get_delta();

					return delta_time;
				}
		};
	}

	class DeltaTime : public impl::DeltaTimeImpl<float, std::uint32_t, engine::time::TimePoint, engine::time::Interval>
	{
		public:
			using Base = DeltaTime::DeltaTimeImpl;

			constexpr DeltaTime(TicksPerSecond ideal_rate, Time current_time, Scalar minimum_delta=DEFAULT_MINIMUM_DELTA) :
				Base(ideal_rate, current_time, minimum_delta)
			{}

			constexpr DeltaTime(TicksPerSecond ideal_rate) :
				DeltaTime(ideal_rate, engine::time::Clock::now())
			{}

			constexpr DeltaTime() :
				DeltaTime(TARGET_FRAMES_PER_SECOND, engine::time::Clock::now())
			{}

			using Base::operator Scalar;
			using Base::operator~;

			constexpr friend DeltaTime& operator<<(DeltaTime& delta_time, Time time)
			{
				delta_time.update(time);

				return delta_time;
			}

			constexpr friend const DeltaTime& operator>>(const DeltaTime& delta_time, Scalar& delta_out)
			{
				delta_out = delta_time.get_delta();

				return delta_time;
			}
	};
}