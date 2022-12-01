// TODO: Rework/deprecate.

#pragma once

#include "types.hpp"

#include <cmath>
#include <functional>

namespace app
{
	class Timer;
	class TimedEventManager;

	using TimerFunction           = Delegate<void(Timer&, Duration)>;
	using ContinuousTimerFunction = Delegate<Duration(Timer&, Duration)>;

	class Timer
	{
		public:
			using Time = Duration;

			static constexpr Duration STOP = -1;
			static constexpr Duration DURATION_SECOND = 1000; // ms

			friend TimedEventManager;
		protected:
			TimerFunction event_fn;

			Time wait_begin_time = 0;
			Duration wait_duration = 0;

			bool trigger_on_destruct = false;
			bool event_triggered = false;

			void trigger(Duration time_spent);

			Timer() = default;
		public:
			static Timer make_continuous(Duration wait_duration, Time now, ContinuousTimerFunction event_fn, bool trigger_on_destruct=false);

			// 'wait_duration' is in milliseconds.
			// 'now' should be the current timestamp.
			// 'event_fn' is the function to be called upon execution of this timer.
			Timer(Duration wait_duration, Time now, TimerFunction event_fn, bool trigger_on_destruct=false);
			~Timer();

			inline bool triggered() const { return event_triggered; }

			// Returns 'true' if this timer has not triggered, and has a valid delegate function.
			inline bool valid() const { return (!triggered()) && (event_fn); }

			// The timestamp this timer was started at.
			inline Time start_time() const { return wait_begin_time; }

			// The duration of this timer.
			inline Duration duration() const { return wait_duration; }

			// Equivalent to 'valid'.
			inline explicit operator bool() const { return valid(); }

			// Retrieves the duration since the creation of this timer.
			Duration time_elapsed(Time now);

			// Checks if the wait-duration has been reached.
			// If the duration has been reached, 'trigger' is called, resulting in the execution of 'event_fn'.
			bool update(Time now);

			// Forces the timer to execute immediately.
			void execute(Time now);

			// Restarts the timer using the previously defined duration.
			void restart(Time now);

			// Restarts the timer using a different duration.
			void restart(Time now, Duration wait_duration);
	};

	class TimedEventManager
	{
		public:
			using Time = Timer::Time;

			bool add_timer(Timer timer);

			void update_timers(Time now);

			// Called automatically by 'update'.
			// Removes completed/invalid timers.
			void flush_timers();

			inline friend TimedEventManager& operator<<(TimedEventManager& event_manager, Time now)
			{
				event_manager.update_timers(now);

				return event_manager;
			}

			inline friend TimedEventManager& operator<<(TimedEventManager& event_manager, Timer timer)
			{
				event_manager.add_timer(std::move(timer));

				return event_manager;
			}
		protected:
			std::vector<Timer> timers;
	};
}