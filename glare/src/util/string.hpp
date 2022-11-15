#pragma once

#include <string>
#include <string_view>
#include <regex>
#include <utility>
#include <optional>
#include <type_traits>

//#include <fmt/core.h>

struct aiString;

namespace util
{
	std::string lowercase(std::string str);
	std::string lowercase(std::string_view str);

	std::string uppercase(std::string str);
	std::string uppercase(std::string_view str);

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

	// This function executes `callback` for every substring between instances of the `separator` specified.
	// e.g. a string "A::B::C" will result in 3 function calls for "A", "B" and "C".
	// 
	// If `callback` returns false, execution will complete immediately.
	// 
	// This function returns true if `str` contains `separator`.
	template <typename Callback>
	bool split(std::string_view str, std::string_view separator, Callback callback)
	{
		std::size_t find_result = 0;

		bool result = false;

		while (true)
		{
			auto begin = (str.begin() + find_result);

			find_result = str.find(separator, find_result);

			if (find_result == std::string_view::npos)
			{
				if constexpr (std::is_same_v<std::invoke_result_t<Callback, std::string_view, bool>, bool>)
				{
					callback(std::string_view{ begin, str.end() }, true);
				}
				else if constexpr (std::is_same_v<std::invoke_result_t<Callback, std::string_view>, bool>)
				{
					callback(std::string_view{ begin, str.end() }, true);
				}
				else
				{
					callback(std::string_view{ begin, str.end() });
				}

				break;
			}
			else
			{
				result = true;

				auto substr = std::string_view { begin, (str.begin() + find_result) };

				if constexpr (std::is_same_v<std::invoke_result_t<Callback, std::string_view, bool>, bool>)
				{
					if (!callback(substr, false))
					{
						break;
					}
				}
				else if constexpr (std::is_same_v<std::invoke_result_t<Callback, std::string_view>, bool>)
				{
					if (!callback(substr))
					{
						break;
					}
				}
				else
				{
					callback(substr);
				}

				find_result += separator.size();
			}
		}

		return result;
	}

	// TODO: Move this to a different header/file.
	std::string_view to_string_view(const aiString& str);
}