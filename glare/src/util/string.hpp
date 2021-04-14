#pragma once

#include <string>
#include <regex>
#include <utility>

namespace util
{
	std::string lowercase(std::string str);
	std::smatch get_regex_groups(const std::string& s, const std::regex& re);
	std::smatch parse_regex(const std::string& str, const std::string& regex_str);

	template <typename First>
	inline std::string concat(First&& f)
	{
		return f;
	}

	template <typename First, typename Second, typename ...Remaining>
	inline std::string concat(First&& first, Second&& second, Remaining&&... strs)
	{
		return first + concat<Second, Remaining...>(std::forward<Second&&>(second), strs...);
	}
}