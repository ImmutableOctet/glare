#include <app/platform/platform_extensions.hpp>

//#define WIN32_LEAN_AND_MEAN 1

#include <windows.h>

#include "timeapi.h"
#include "synchapi.h"

#include <chrono>

namespace app
{
	namespace platform
	{
		namespace impl
		{
			static constexpr auto win32_high_precision_timer_resolution = static_cast<UINT>(1u); // ms
		}

		bool enable_high_precision_timers()
		{
			const auto result = timeBeginPeriod(impl::win32_high_precision_timer_resolution);

			return (result == TIMERR_NOERROR);
		}

		bool disable_high_precision_timers()
		{
			const auto result = timeEndPeriod(impl::win32_high_precision_timer_resolution);

			return (result == TIMERR_NOERROR);
		}

		void sleep_ms(Milliseconds sleep_time)
		{
			// STL:
			//std::this_thread::sleep_for(std::chrono::milliseconds { sleep_time });
				
			// Alternative implementation:
			//std::this_thread::sleep_until(sleep_begin + std::chrono::milliseconds { sleep_time });
				
			// Windows API:
			//SleepEx(static_cast<DWORD>(sleep_time), FALSE);

			// Alternative implementation:
			Sleep(static_cast<DWORD>(sleep_time));
		}
	}
}