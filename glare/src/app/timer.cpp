#include "timer.hpp"

#include <algorithm>

namespace app
{
	// Timer:
	Timer::Timer(Duration wait_duration, Time now, TimerFunction event_fn, bool trigger_on_destruct)
	: wait_duration(wait_duration), wait_begin_time(now), event_fn(std::move(event_fn)), trigger_on_destruct(trigger_on_destruct){}

	Timer::~Timer()
	{
		if (trigger_on_destruct)
		{
			if (!triggered())
			{
				trigger(0);
			}
		}
	}

	Duration Timer::time_elapsed(Time now)
	{
		return std::abs((now - start_time()));
	}

	void Timer::trigger(Duration time_spent)
	{
		if (event_fn)
		{
			event_fn(*this, time_spent);
		}
	}

	Timer Timer::make_continuous(Duration wait_duration, Time now, ContinuousTimerFunction event_fn, bool trigger_on_destruct)
	{
		///assert(event_fn);

		return Timer(wait_duration, now, [event_fn](Timer& timer, Duration time_elapsed)
		{
			auto new_duration = event_fn(timer, time_elapsed);

			if (new_duration != STOP)
			{
				timer.restart((timer.start_time() + time_elapsed), new_duration);
			}
		}, trigger_on_destruct);
	}

	bool Timer::update(Time now)
	{
		auto time_spent = time_elapsed(now);

		if (time_spent >= duration())
		{
			trigger(time_spent);

			return true;
		}

		return false;
	}
	
	void Timer::execute(Time now)
	{
		trigger(time_elapsed(now));
	}
	
	void Timer::restart(Time now)
	{
		this->wait_begin_time = now;
		this->event_triggered = false;
	}

	void Timer::restart(Time now, Duration wait_duration)
	{
		this->wait_duration = wait_duration;

		restart(now);
	}
	
	// TimedEventManager:
	bool TimedEventManager::add_timer(Timer timer)
	{
		if (!timer)
		{
			return false;
		}

		timers.push_back(timer);

		return true;
	}
	
	void TimedEventManager::update_timers(Time now)
	{
		for (auto& timer : timers)
		{
			timer.update(now);
		}

		flush_timers();
	}

	void TimedEventManager::flush_timers()
	{
		timers.erase
		(
			std::remove_if(timers.begin(), timers.end(), [](const Timer& timer)
			{
				return (!timer.valid());
			}),

			timers.end()
		);
	}
}