#pragma once

#include "network_time.hpp"

#include <engine/timer_impl.hpp>

namespace engine
{
	class NetworkTimer : public impl::TimerImpl<engine::network::time::Clock, engine::network::time::TimePoint, engine::network::time::Duration>
	{
		public:
			using Base = NetworkTimer::TimerImpl;

			static decltype(auto) to_duration(auto&&... args)
			{
				return engine::network::time::to_duration(std::forward<decltype(args)>(args)...);
			}

			NetworkTimer() = default;

			NetworkTimer(auto&& length, bool start_immediately=true) :
				Base
				(
					std::forward<decltype(length)>(length),
					[](auto&&... args) { return to_duration(std::forward<decltype(args)>(args)...); },
					start_immediately
				)
			{}

			using Base::operator();
			using Base::operator bool;

			friend auto operator<=>(const NetworkTimer&, const NetworkTimer&) = default;
	};

	namespace network
	{
		namespace time
		{
			//using Timer = engine::NetworkTimer;
		}

		using NetworkTimer = engine::NetworkTimer; // engine::network::time::Timer;
	}
}