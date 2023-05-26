#pragma once

#include "types.hpp"

#include <cmath>
#include <array>

namespace app
{
	static constexpr std::size_t DEFAULT_DELTALOG_SIZE = 20;

	template <typename ScalarType=float, std::size_t log_size=DEFAULT_DELTALOG_SIZE>
	class DeltaTimeImpl
	{
		public:
			using Scalar = ScalarType;
			using Rate = unsigned int;
			using Time = Milliseconds;
			using IndexType = int; // std::size_t;
			using Interval = Scalar;

			static constexpr Interval calculate_interval(Rate ideal_rate)
			{
				if (ideal_rate == 0)
				{
					return {};
				}

				return (static_cast<Interval>(ideal_rate) * INTERVAL_SECOND);
			}

			static constexpr Scalar get_inv_delta(Scalar delta)
			{
				return (static_cast <Scalar>(1.0) / delta);
			}

		protected:
			static constexpr Scalar LOG_SCALAR = ((static_cast<Scalar>(1.5) / static_cast<Scalar>(log_size)) * static_cast<Scalar>(0.75));
			
			static constexpr Scalar DEFAULT_DELTA = static_cast<Scalar>(1.0);
			static constexpr Scalar DEFAULT_INV_DELTA = get_inv_delta(DEFAULT_DELTA);
			static constexpr Scalar DEFAULT_MINIMUM_DELTA = static_cast<Scalar>(0.0);

			static constexpr Interval INTERVAL_SECOND = (static_cast<Interval>(1) / static_cast <Interval>(1000));
			static constexpr IndexType DEFAULT_NODE = static_cast<IndexType>(0);
			
			Rate ideal_rate;
			Interval ideal_interval;

			Time time_previous_frame;
			Time time_current_frame;

			Scalar delta = DEFAULT_DELTA;
			Scalar inv_delta = DEFAULT_INV_DELTA;
			Scalar minimum_delta = DEFAULT_MINIMUM_DELTA;

			IndexType delta_node = DEFAULT_NODE;

			std::array<Scalar, log_size> delta_log;

		public:
			DeltaTimeImpl(Rate ideal_rate, Time time_milliseconds=0, Scalar minimum_delta=DEFAULT_MINIMUM_DELTA)
				: minimum_delta(minimum_delta)
			{
				set_rate(ideal_rate);
				reset(time_milliseconds);
			}

			operator Scalar() const  { return delta;     }
			Scalar operator~() const { return inv_delta; }

			constexpr std::size_t size() const { return log_size; }

			Scalar   get_delta()         const { return delta;          }
			Scalar   get_inv_delta()     const { return inv_delta;      }
			Interval get_interval()      const { return ideal_interval; }
			Rate     get_rate()          const { return ideal_rate;     }

			Time    current_frame_time() const { return time_current_frame; }

			template <typename T>
			T per_frame(const T& frame_diff)
			{
				return (frame_diff * delta);
			}

			void set_rate(Rate ideal_rate)
			{
				this->ideal_rate = ideal_rate;
				this->ideal_interval = calculate_interval(this->ideal_rate);
			}

			void reset_log()
			{
				for (auto& delta_value : delta_log)
				{
					delta_value = DEFAULT_DELTA;
				}
			}

			void reset(Time time, bool catch_up=false)
			{
				time_previous_frame = ((catch_up) ? time_current_frame : time);

				time_current_frame = time;
				delta = DEFAULT_DELTA;
				delta_node = DEFAULT_NODE;

				reset_log();
			}

			void reset(Rate ideal_rate, Time time, bool catch_up=false)
			{
				set_rate(ideal_rate);

				reset(time, catch_up);
			}

			Scalar update(Time time)
			{
				time_previous_frame = time_current_frame;
				time_current_frame = time;

				auto frame_time = ((time_current_frame - time_previous_frame) * ideal_interval);

				delta_log[delta_node] = frame_time;

				delta_node = ((delta_node + 1) % size());

				Scalar value = {};

				for (auto& delta_value : delta_log)
				{
					value += delta_value;
				}

				delta = std::max((value * LOG_SCALAR), minimum_delta);
				inv_delta = get_inv_delta(delta);

				return delta;
			}

			friend DeltaTimeImpl& operator<<(DeltaTimeImpl& delta_time, Time time)
			{
				delta_time.update(time);

				return delta_time;
			}

			friend DeltaTimeImpl& operator>>(DeltaTimeImpl& delta_time, Scalar& delta_out)
			{
				delta_out = delta_time.get_delta();

				return delta_time;
			}
	};

	using DeltaTime = DeltaTimeImpl<float, 20U>;
}