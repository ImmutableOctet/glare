#pragma once

#include <string>
#include <regex>

namespace util
{
	std::string lowercase(std::string str);
	std::smatch get_regex_groups(const std::string& s, const std::regex& re);
	std::smatch parse_regex(const std::string& str, const std::string& regex_str);
}