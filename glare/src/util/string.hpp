#pragma once

#include <string>
#include <string_view>
#include <regex>
#include <utility>
#include <optional>
#include <type_traits>
#include <charconv>

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

	// Reconstructs `old_view` using `new_basis` for storage.
	inline std::string_view remap_string_view(std::string_view old_basis, std::string_view new_basis, std::string_view old_view)
	{
		if (old_view.empty())
		{
			return {};
		}

		const auto offset = (old_view.data() - old_basis.data());
		const auto length = old_view.size(); // length();

		//assert(new_basis.size() >= (length + offset));

		if (new_basis.size() < (length + offset))
		{
			return {};
		}

		return { (new_basis.data() + offset), length };
	}

	inline std::string_view trim(std::string_view str, std::string_view trim_values=" \n")
	{
		if (trim_values.empty())
		{
			return str;
		}

		str.remove_prefix(str.find_first_not_of(trim_values));
		str.remove_suffix((str.length() - str.find_last_not_of(trim_values) - 1));

		return str;
	}

	inline std::string_view match_view(std::string_view str, const std::smatch& matches, std::size_t index, std::string_view trim_values=" \n")
	{
		if (index >= matches.size())
		{
			return {};
		}

		if (!matches[index].matched)
		{
			return {};
		}

		if (matches.length(index) == 0)
		{
			return {};
		}

		return trim
		(
			std::string_view
			{
				(str.data() + matches.position(index)),
				static_cast<std::size_t>(matches.length(index))
			},

			trim_values
		);
	}

	inline std::string quote(std::string_view str)
	{
		//return fmt::format("\"{}\"", str);

		return "\"" + std::string(str) + "\"";
	}

	template <typename T>
	inline bool from_string(std::string_view str, T& out)
	{
		auto result = std::from_chars(str.data(), (str.data() + str.size()), out);

		return (result.ec == std::errc());
	}

	template <typename T>
	inline std::optional<T> from_string(std::string_view str)
	{
		if (T out; from_string(str, out)) // auto out = T{};
		{
			return out;
		}

		return std::nullopt;
	}

	template <typename StringType=std::string_view, typename CharType=typename StringType::value_type>
	inline bool is_quoted(const StringType& str, CharType quote_char='"')
	{
		return

		/*
			NOTE: This check covers edge-cases where too small of a string is provided.
			
			This also means the single-character string `"` is not considered quoted,
			since it doesn't meet the "quote on both sides" requirement.
			
			This is important, since it should always be valid to call `unquote` or `remove_quotes` on a quoted string.
		*/
		(str.length() >= 2)
		&&
		(
			str.starts_with(quote_char)
			&&
			str.ends_with(quote_char)
		);
	}

	template <typename StringType=std::string_view, typename CharType=typename StringType::value_type, typename... Chars>
	inline bool is_quoted(const StringType& str, CharType quote_char, Chars ...quote_chars)
	{
		/*
		if (str.length() < 2)
		{
			return false;
		}
		*/

		// TODO: Optimize length check.
		return is_quoted(str, quote_char) || is_quoted(str, std::forward<Chars>(quote_chars)...);
	}

	// Removes leading and trailing quotation marks. (One on each side)
	// 
	// NOTES:
	// * This function does not check which quotation symbol is used.
	// * This function does not check for `is_quoted` before trimming.
	// 
	// See also: `unquote_safe`
	template <typename StringTypeIn=std::string_view, typename StringTypeOut=StringTypeIn> // std::string_view
	inline StringTypeOut unquote(const StringTypeIn& str, std::size_t quote_symbol_length=1)
	{
		return StringTypeOut { (str.data() + (quote_symbol_length)), (str.size() - (quote_symbol_length * 2)) };
	}

	// Safely removes leading and trailing quotes from a 'quoted' string. (One on each side)
	// 
	// NOTE: Performs check using `is_quoted` before trimming `str`.
	template <typename StringTypeIn=std::string_view, typename StringTypeOut=StringTypeIn> // std::string_view
	inline StringTypeOut unquote_safe(const StringTypeIn& str)
	{
		if (is_quoted(str))
		{
			return unquote<StringTypeIn, StringTypeOut>(str);
		}
		
		return StringTypeOut(str);
	}

	// Removes quotes from an `std::string` instance.
	// 
	// NOTE: This function does not perform length or quotation symbol checks.
	// To perform these checks, use `remove_quotes_safe` instead.
	inline std::string& remove_quotes(std::string& str)
	{
		str.erase(0, 1);
		str.erase(str.length() - 1);

		return str;
	}

	inline std::string& remove_quotes_safe(std::string& str)
	{
		// NOTE: Length check performed inside `is_quoted`.
		if (!is_quoted(std::string_view(str)))
		{
			return str;
		}

		return remove_quotes(str);
	}

	// TODO: Optimize.
	template <typename First>
	inline std::string concat(First&& f)
	{
		return f;
	}

	// TODO: Optimize.
	template <typename First, typename Second, typename ...Remaining>
	inline std::string concat(First&& first, Second&& second, Remaining&&... strs)
	{
		return first + concat<Second, Remaining...>(std::forward<Second&&>(second), strs...);
	}

	// Wraps a string object inside an `std::optional`, where `std::nullopt` is returned only on empty strings.
	template <typename StringType=std::string>
	inline std::optional<StringType> opt_str(const StringType& str)
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
	bool split(std::string_view str, std::string_view separator, Callback callback, std::string_view trim_values=" \n")
	{
		std::size_t find_result = 0;

		bool result = false;

		while (true)
		{
			auto begin = (str.begin() + find_result);

			find_result = str.find(separator, find_result);

			if (find_result == std::string_view::npos)
			{
				auto substr = std::string_view{ begin, str.end() };

				substr = trim(substr, trim_values);

				if constexpr (std::is_same_v<std::invoke_result_t<Callback, std::string_view, bool>, bool>)
				{
					callback(substr, true);
				}
				else if constexpr (std::is_same_v<std::invoke_result_t<Callback, std::string_view>, bool>)
				{
					callback(substr, true);
				}
				else
				{
					callback(substr);
				}

				break;
			}
			else
			{
				result = true;

				auto substr = std::string_view { begin, (str.begin() + find_result) };

				substr = trim(substr, trim_values);

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

	// TODO: Move this to a different source file.
	std::string_view to_string_view(const aiString& str);
}