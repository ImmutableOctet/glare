#pragma once

#include "time.hpp"
#include "timer_impl.hpp"

namespace engine
{
	class Timer : public impl::TimerImpl<engine::time::Clock, engine::time::TimePoint, engine::time::Duration>
	{
		public:
			using Base = Timer::TimerImpl;

			static decltype(auto) to_duration(auto&&... args)
			{
				return engine::time::to_duration(std::forward<decltype(args)>(args)...);
			}

			Timer() = default;

			Timer(auto&& length, bool start_immediately=true) :
				Base
				(
					std::forward<decltype(length)>(length),
					[](auto&&... args) { return to_duration(std::forward<decltype(args)>(args)...); },
					start_immediately
				)
			{}

			using Base::operator();
			using Base::operator bool;

			friend auto operator<=>(const Timer&, const Timer&) = default;
	};
}