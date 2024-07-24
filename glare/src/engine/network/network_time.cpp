#include "network_time.hpp"

#include <engine/time.hpp>
#include <engine/time_impl.hpp>

namespace engine
{
	namespace network
	{
		namespace time
		{
			std::optional<Duration> to_duration(DurationRaw duration_rep, StringHash time_symbol_id)
			{
				return engine::time::shared::impl::to_duration_impl<Duration>(duration_rep, time_symbol_id);
			}

			std::optional<Duration> to_duration(DurationRaw duration_rep, std::string_view time_symbol)
			{
				return engine::time::shared::impl::to_duration_dispatch_impl(duration_rep, time_symbol, [](auto&&... args) { return to_duration(std::forward<decltype(args)>(args)...); });
			}

			std::optional<Duration> to_duration(float duration_raw, StringHash time_symbol_id)
			{
				return engine::time::shared::impl::to_duration_float_impl<Duration>(duration_raw, time_symbol_id, [](auto&&... args) { return to_duration(std::forward<decltype(args)>(args)...); });
			}

			std::optional<Duration> to_duration(double duration_raw, StringHash time_symbol_id)
			{
				return engine::time::shared::impl::to_duration_float_impl<Duration>(duration_raw, time_symbol_id, [](auto&&... args) { return to_duration(std::forward<decltype(args)>(args)...); });
			}

			std::optional<Duration> to_duration(float duration_raw, std::string_view time_symbol)
			{
				return engine::time::shared::impl::to_duration_dispatch_impl(duration_raw, time_symbol, [](auto&&... args) { return to_duration(std::forward<decltype(args)>(args)...); });
			}

			std::optional<Duration> to_duration(double duration_raw, std::string_view time_symbol)
			{
				return engine::time::shared::impl::to_duration_dispatch_impl(duration_raw, time_symbol, [](auto&&... args) { return to_duration(std::forward<decltype(args)>(args)...); });
			}
		}
	}
}