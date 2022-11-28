#include "timer.hpp"

namespace engine
{
	Timer::Timer(Duration length, bool start_immediately)
		: length(length)
	{
		if (start_immediately)
		{
			start();
		}
	}

	bool Timer::start(bool restart)
	{
		if (!restart)
		{
			if (paused())
			{
				return resume();
			}

			if (started())
			{
				return false;
			}
		}

		const auto now = Clock::now();

		start_point = now;
		pause_point = std::nullopt;
		end_point = (now + length);

		return true; // started();
	}

	bool Timer::start(Duration length)
	{
		if (started())
		{
			return false;
		}

		this->length = length;

		return start(true);
	}

	bool Timer::restart()
	{
		return start(true);
	}

	bool Timer::restart(Duration length)
	{
		return start(length);
	}

	bool Timer::pause()
	{
		if (paused())
		{
			return false;
		}

		pause_point = Clock::now();

		return paused();
	}

	bool Timer::resume()
	{
		if (!paused())
		{
			return false;
		}

		const auto now = Clock::now();

		const auto prev_elapsed = (start_point) ? (*pause_point - *start_point) : Duration {};
		const auto remaining_length = (length - prev_elapsed);

		pause_point = std::nullopt;
		start_point = now;
		end_point   = (now + remaining_length);

		return true;
	}

	bool Timer::stop()
	{
		if (stopped())
		{
			return false;
		}

		start_point = std::nullopt;
		pause_point = std::nullopt;
		end_point   = std::nullopt;

		return true; // stopped();
	}

	bool Timer::clear()
	{
		if (stop())
		{
			length = {};

			return true;
		}

		return false;
	}

	bool Timer::set_duration(Duration length)
	{
		this->length = length;

		if (paused())
		{
			const auto now = Clock::now();

			start_point = now; // std::nullopt;
			pause_point = now;

			// Projected end point updated for accuracy when comparing against new length.
			// (Practically speaking, this shouldn't matter for start/stop/pause behavior)
			end_point   = (now + length);

			return true;
		}

		if (started())
		{
			// Restart with new length.
			return start(length);
		}

		// Not yet started, just the length assignment will do.
		return true;
	}

	bool Timer::paused() const
	{
		return pause_point.has_value();
	}

	bool Timer::has_projected_end() const
	{
		return end_point.has_value();
	}

	bool Timer::completed() const
	{
		if (stopped())
		{
			return false;
		}

		return (remaining() <= Duration(0));
	}

	bool Timer::started() const
	{
		return start_point.has_value();
	}

	Timer::Duration Timer::duration() const
	{
		return length;
	}

	Timer::Milliseconds Timer::milliseconds() const
	{
		return std::chrono::duration_cast<Milliseconds>(duration());
	}

	Timer::Duration Timer::elapsed() const
	{
		if (!has_projected_end())
		{
			return {};
		}

		if (paused())
		{
			//return (*end_point - *pause_point);
			return (*pause_point - *start_point);
		}

		const auto now = Clock::now();

		return (now - *start_point); // *end_point - ...
	}

	Timer::Milliseconds Timer::milliseconds_elapsed() const
	{
		return std::chrono::duration_cast<Milliseconds>(elapsed());
	}

	Timer::Duration Timer::remaining() const
	{
		//return (length - elapsed());

		if (!has_projected_end())
		{
			return length;
		}

		if (paused())
		{
			return (length - elapsed());
		}

		const auto now = Clock::now();

		return (*end_point - now);
	}

	Timer::Milliseconds Timer::milliseconds_remaining() const
	{
		return std::chrono::duration_cast<Milliseconds>(remaining());
	}
}