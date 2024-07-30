#pragma once

#include <app/types.hpp>

namespace app
{
	namespace platform
	{
		bool enable_high_precision_timers();
		bool disable_high_precision_timers();

		void sleep_ms(Milliseconds sleep_time);

		template <typename ClockType, typename DurationType, typename MillisecondsRaw>
		inline void sleep(DurationType sleep_time, MillisecondsRaw sleep_time_ms, bool allow_spin_lock=false, bool allow_short_sleep=true, bool sleep_on_one_ms=false)
		{
			auto actual_sleep_time_ms = sleep_time_ms;

			const auto can_short_sleep = ((allow_short_sleep) && (sleep_time_ms > 1));

			if (can_short_sleep)
			{
				actual_sleep_time_ms -= 1;
			}

			if (allow_spin_lock)
			{
				using clock = ClockType;

				const auto sleep_begin = clock::now();

				const auto expected_end_point = (sleep_begin + sleep_time);

				if (can_short_sleep)
				{
					sleep_ms(actual_sleep_time_ms);
				}
				else if (sleep_on_one_ms)
				{
					sleep_ms(static_cast<Milliseconds>(1));
				}

				while (clock::now() < expected_end_point) {}
			}
			else
			{
				sleep_ms(actual_sleep_time_ms);
			}
		}
	}
}