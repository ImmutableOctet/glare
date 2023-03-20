#include "timer.hpp"

#include "meta/meta.hpp"

#include <ratio>

namespace engine
{
	static constexpr Timer::Duration to_duration_seconds_impl(auto seconds)
	{
		return std::chrono::duration_cast<Timer::Duration>(std::chrono::round<std::chrono::nanoseconds>(seconds));
	}

	template <typename FloatType>
	static std::optional<Timer::Duration> to_duration_float_impl(FloatType duration_raw, StringHash time_symbol_id)
	{
		using namespace engine::literals;

		auto convert = [duration_raw]<typename OutType, typename RatioType=typename OutType::period>()
		{
			constexpr auto duration_ratio = (static_cast<FloatType>(Timer::Duration::period::num) / static_cast<FloatType>(Timer::Duration::period::den));
			constexpr auto local_scale_ratio = (static_cast<FloatType>(RatioType::num) / static_cast<FloatType>(RatioType::den));
			constexpr auto time_symbol_inversion_ratio = (local_scale_ratio / duration_ratio);

			auto raw_out = static_cast<OutType::rep>(duration_raw * time_symbol_inversion_ratio);

			return Timer::Duration(raw_out);
		};

		switch (time_symbol_id)
		{
			case "d"_hs:
				return convert.operator()<Timer::Days>();
			case "h"_hs:
				return convert.operator()<Timer::Hours>();
			case "m"_hs:
				return convert.operator()<Timer::Minutes>();
			case "s"_hs:
				return Timer::to_duration(duration_raw); // return convert.operator()<Timer::Seconds>();
			case "ms"_hs:
				return convert.operator()<Timer::Milliseconds>(); // std::milli
			case "us"_hs:
				return convert.operator()<Timer::Microseconds>();
		}

		return std::nullopt;
	}

	template <typename DurationTypeRaw>
	static std::optional<Timer::Duration> to_duration_dispatch_impl(DurationTypeRaw duration_raw, std::string_view time_symbol)
	{
		using namespace engine::literals;

		const auto time_symbol_id = (time_symbol.empty())
			? "s"_hs
			: hash(time_symbol)
		;

		return Timer::to_duration(duration_raw, time_symbol_id);
	}

	std::optional<Timer::Duration> Timer::to_duration(DurationRaw duration_rep, StringHash time_symbol_id)
	{
		using namespace engine::literals;

		switch (time_symbol_id)
		{
			case "d"_hs:
				return Timer::Days(duration_rep);
			case "h"_hs:
				return Timer::Hours(duration_rep);
			case "m"_hs:
				return Timer::Minutes(duration_rep);
			case "s"_hs:
				return Timer::Seconds(duration_rep);
			case "ms"_hs:
				return Timer::Milliseconds(duration_rep);
			case "us"_hs:
				return Timer::Microseconds(duration_rep);
		}

		return std::nullopt;
	}

	std::optional<Timer::Duration> Timer::to_duration(DurationRaw duration_rep, std::string_view time_symbol)
	{
		return to_duration_dispatch_impl(duration_rep, time_symbol);
	}

	std::optional<Timer::Duration> Timer::to_duration(float duration_raw, StringHash time_symbol_id)
	{
		return to_duration_float_impl(duration_raw, time_symbol_id);
	}

	std::optional<Timer::Duration> Timer::to_duration(double duration_raw, StringHash time_symbol_id)
	{
		return to_duration_float_impl(duration_raw, time_symbol_id);
	}

	std::optional<Timer::Duration> Timer::to_duration(float duration_raw, std::string_view time_symbol)
	{
		return to_duration_dispatch_impl(duration_raw, time_symbol);
	}

	std::optional<Timer::Duration> Timer::to_duration(double duration_raw, std::string_view time_symbol)
	{
		return to_duration_dispatch_impl(duration_raw, time_symbol);
	}

	Timer::Duration Timer::to_duration(FloatSeconds seconds)
	{
		return to_duration_seconds_impl(seconds);
	}

	Timer::Duration Timer::to_duration(float seconds)
	{
		return to_duration(FloatSeconds(seconds));
	}

	Timer::Duration Timer::to_duration(DoubleSeconds seconds)
	{
		return to_duration_seconds_impl(seconds);
	}

	Timer::Duration Timer::to_duration(double seconds)
	{
		return to_duration(DoubleSeconds(seconds));
	}

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

	bool Timer::active() const
	{
		return (started() && !paused());
	}

	bool Timer::stopped() const
	{
		return !started();
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

	std::optional<Timer::TimePoint> Timer::get_start_point() const { return start_point; }
	std::optional<Timer::TimePoint> Timer::get_pause_point() const { return pause_point; }
	std::optional<Timer::TimePoint> Timer::get_projected_end_point() const { return end_point; }
}