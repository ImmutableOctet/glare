#pragma once

#include "meta/types.hpp"

#include "timer.hpp"

namespace engine
{
	// Abstraction for timing-dependent event dispatch.
	// 
	// The object instance enclosed in `event_instance` will be triggered or enqueued
	// as an event once the `delay` timer has been completed.
	struct TimedEvent
	{
		Timer delay;

		MetaAny event_instance;

		inline bool completed() const
		{
			return delay.completed();
		}

		inline explicit operator bool() const
		{
			return completed();
		}

		auto type() const
		{
			return event_instance.type();
		}

		auto type_id() const
		{
			return type().id();
		}
	};
}