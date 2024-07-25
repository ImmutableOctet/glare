#pragma once

#include "time_decl.hpp"

#include "types.hpp"

#include <util/type_traits.hpp>

#include <chrono>
#include <optional>
#include <string_view>
#include <type_traits>

namespace engine
{
	namespace time
	{
		namespace shared
		{
			template <typename T>
			inline static constexpr bool IsDurationType = util::is_specialization_v<T, std::chrono::duration>;
		}
	}

	namespace time
	{
		template
		<
			typename DurationType,

			std::enable_if<engine::time::shared::IsDurationType<DurationType>, int>::type=0
		>
		constexpr Duration to_duration(DurationType duration_value)
		{
			return engine::time::impl::duration_cast<DurationType>(duration_value);
		}

		std::optional<Duration> to_duration(DurationRaw duration_rep, StringHash time_symbol_id);
		std::optional<Duration> to_duration(DurationRaw duration_rep, std::string_view time_symbol);

		std::optional<Duration> to_duration(float duration_raw,  StringHash time_symbol_id);
		std::optional<Duration> to_duration(double duration_raw, StringHash time_symbol_id);

		std::optional<Duration> to_duration(double duration_raw, std::string_view time_symbol);
		std::optional<Duration> to_duration(float duration_raw,  std::string_view time_symbol);
	}
}