#pragma once

#include <string>
#include <string_view>
#include <regex>
#include <utility>
#include <optional>

//#include <fmt/core.h>

struct aiString;

namespace util
{
	std::string lowercase(std::string str);
	std::string lowercase(std::string_view str);

	std::smatch get_regex_groups(const std::string& s, const std::regex& re);
	std::smatch parse_regex(const std::string& str, const std::string& regex_str);

	inline std::string quote(const std::string& str)
	{
		//return fmt::format("\"{}\"", str);

		return "\"" + str + "\"";
	}

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

	inline std::optional<std::string> opt_str(const std::string& str)
	{
		if (str.empty())
		{
			return std::nullopt;
		}

		return str;
	}

	std::string_view to_string_view(const aiString& str);
}