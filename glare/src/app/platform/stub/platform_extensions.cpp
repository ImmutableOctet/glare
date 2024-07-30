#include <app/platform/platform_extensions.hpp>

#include <chrono>
#include <thread>

namespace app
{
	namespace platform
	{
		bool enable_high_precision_timers()
		{
			return true;
		}

		bool disable_high_precision_timers()
		{
			return true;
		}

		void sleep_ms(Milliseconds sleep_time)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds { sleep_time });
		}
	}
}