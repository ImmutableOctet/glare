#pragma once

#include <engine/time.hpp>

#include "network_time_decl.hpp"

namespace engine
{
	namespace network
	{
		namespace time
		{
			template
			<
				typename DurationType,

				std::enable_if<engine::time::shared::IsDurationType<DurationType>, int>::type=0
			>
			Duration to_duration(DurationType duration_value)
			{
				return engine::network::time::impl::duration_cast<DurationType>(duration_value);
			}

			std::optional<Duration> to_duration(DurationRaw duration_rep, StringHash time_symbol_id);
			std::optional<Duration> to_duration(DurationRaw duration_rep, std::string_view time_symbol);

			std::optional<Duration> to_duration(float duration_raw,  StringHash time_symbol_id);
			std::optional<Duration> to_duration(double duration_raw, StringHash time_symbol_id);

			std::optional<Duration> to_duration(double duration_raw, std::string_view time_symbol);
			std::optional<Duration> to_duration(float duration_raw,  std::string_view time_symbol);
		}
	}
}