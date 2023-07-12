#pragma once

#include <string>
#include <filesystem>

#include "log.hpp"

namespace util
{
	std::string load_string(const std::string& path);
	std::string load_string(const std::filesystem::path& path);

	void save_string(const std::string& value, const std::string& path);
	void save_string(const std::string& value, const std::filesystem::path& path);
}