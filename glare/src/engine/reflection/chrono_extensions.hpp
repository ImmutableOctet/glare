#pragma once

#include <engine/timer.hpp>
#include <engine/entity/parse.hpp>

#include <string>
#include <string_view>
#include <chrono>
#include <cstddef>

namespace engine::impl
{
	Timer::Duration duration_from_integer(std::int32_t seconds)
	{
		return Timer::to_duration(static_cast<float>(seconds));
	}

	Timer::Duration duration_from_string_view(std::string_view time_expr)
	{
		if (auto duration = parse_time_duration(time_expr))
		{
			return *duration;
		}

		return {};
	}

	Timer::Duration duration_from_string(const std::string& time_expr)
	{
		return duration_from_string_view(time_expr);
	}
}