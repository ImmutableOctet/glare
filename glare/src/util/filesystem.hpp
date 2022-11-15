#pragma once

#include <filesystem>

namespace util
{
	inline std::filesystem::path resolve_paths(std::filesystem::path paths...)
	{
		if (std::filesystem::exists(paths))
		{
			return paths;
		}

		return {};
	}
}