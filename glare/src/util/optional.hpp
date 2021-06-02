#pragma once

#include <optional>

namespace util
{
	template <typename T, typename FallbackFn>
	T value_or(const std::optional<T>& opt, FallbackFn&& fallback)
	{
		if (opt.has_value())
		{
			return *opt;
		}

		return fallback();
	}
}