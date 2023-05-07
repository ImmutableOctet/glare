#pragma once

#include "small_vector.hpp"
//#include "format.hpp"

#include <string>
#include <string_view>
#include <tuple>
#include <array>
#include <regex>
#include <utility>
#include <optional>
#include <type_traits>
#include <charconv>

struct aiString;

namespace util
{
	inline constexpr std::string_view whitespace_symbols = " \t\n\v\f\r";

	std::string lowercase(std::string str);
	std::string lowercase(std::string_view str);

	std::string uppercase(std::string str);
	std::string uppercase(std::string_view str);

	std::smatch get_regex_groups(const std::string& s, const std::regex& re);
	std::smatch parse_regex(const std::string& str, const std::string& regex_str);

	// Reconstructs `old_view` using `new_basis` for storage.
	std::string_view remap_string_view(std::string_view old_basis, std::string_view new_basis, std::string_view old_view);

	std::string_view trim_beginning(std::string_view str, std::string_view trim_values=whitespace_symbols);
    std::string_view trim_ending(std::string_view str, std::string_view trim_values=whitespace_symbols);
	std::string_view trim(std::string_view str, std::string_view trim_values=whitespace_symbols, bool empty_on_only_trim_values=false);
	std::string_view trim(std::string_view str, bool empty_on_only_whitespace);

	bool is_whitespace(std::string_view str, std::string_view whitespace_values=whitespace_symbols);
	bool is_whitespace(char symbol, std::string_view whitespace_values=whitespace_symbols);

	std::string_view match_view(std::string_view str, const std::smatch& matches, std::size_t index, std::string_view trim_values=" \n");

	std::string quote(std::string_view str);

	template <typename T>
	inline bool from_string(std::string_view str, T& out, bool exact_numeric_format=true)
	{
		if (str.empty())
		{
			return false;
		}

		// Disallow decimal points for integral types when using `exact_numeric_format`:
		if constexpr (std::is_integral_v<T>)
		{
			if (exact_numeric_format)
			{
				if (str.contains('.'))
				{
					return false;
				}
			}
		}

		auto result = std::from_chars(str.data(), (str.data() + str.size()), out);

		return (result.ec == std::errc());
	}

	template <>
	inline bool from_string(std::string_view str, bool& out, bool exact_numeric_format)
	{
		if (str.empty())
		{
			return false;
		}

		const auto initial_char = str[0];

		switch (initial_char)
		{
			case 't':
			case 'T':
			case '1':
			case 'y':
			case 'Y':
				out = true;

				break;

			default:
				out = false;

				break;
		}

		return true;
	}

	template <typename T>
	inline std::optional<T> from_string(std::string_view str, bool exact_numeric_format=true)
	{
		if (T out; from_string(str, out, exact_numeric_format)) // auto out = T{};
		{
			return out;
		}

		return std::nullopt;
	}

	// Similar to `is_quoted`, but does not handle scenarios where escaped quotes are used.
	template <typename StringType=std::string_view, typename CharType=typename StringType::value_type>
	bool simplified_is_quoted(const StringType& str, CharType quote_char='"')
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
	bool simplified_is_quoted(const StringType& str, CharType quote_char, Chars... quote_chars)
	{
		/*
		if (str.length() < 2)
		{
			return false;
		}
		*/

		// TODO: Optimize length check.
		return simplified_is_quoted(str, quote_char) || simplified_is_quoted(str, std::forward<Chars>(quote_chars)...);
	}

	bool is_quoted(std::string_view str, char quote_char='"');

	template <typename CharType, typename... Chars>
	bool is_quoted(std::string_view str, CharType quote_char, Chars... quote_chars)
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
	std::string& remove_quotes(std::string& str);

	std::string& remove_quotes_safe(std::string& str);

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
	template <std::size_t n_separators, typename Callback>
	bool split
	(
		std::string_view str,
		const std::array<std::string_view, n_separators>& separators,
		Callback&& callback,
		bool separator_required=false,
		std::string_view trim_values=util::whitespace_symbols,
		bool include_empty_substrings=false
	)
	{
		if (str.empty())
		{
			return false;
		}

		std::size_t current_position = 0;

		bool separator_found = false;

		bool is_first_symbol = true;
		bool is_last_symbol  = false;

		auto execute_callback = [&callback, &is_first_symbol, &is_last_symbol](std::string_view& substr) -> bool
		{
			if constexpr (std::is_invocable_r_v<bool, Callback, std::string_view, bool, bool> || std::is_invocable_r_v<bool, Callback, std::string_view&, bool, bool>)
			{
				if (!callback(substr, is_first_symbol, is_last_symbol))
				{
					return false;
				}
			}
			else if constexpr (std::is_invocable_r_v<void, Callback, std::string_view, bool, bool>)
			{
				callback(substr, is_first_symbol, is_last_symbol);
			}
			else if constexpr (std::is_invocable_r_v<bool, Callback, std::string_view, bool> || std::is_invocable_r_v<bool, Callback, std::string_view&, bool>)
			{
				if (!callback(substr, is_last_symbol))
				{
					return false;
				}
			}
			else if constexpr (std::is_invocable_r_v<void, Callback, std::string_view, bool>)
			{
				callback(substr, is_last_symbol);
			}
			else if constexpr (std::is_invocable_r_v<bool, Callback, std::string_view>)
			{
				if (!callback(substr))
				{
					return false;
				}
			}
			else if constexpr (std::is_invocable_r_v<void, Callback, std::string_view>)
			{
				callback(substr);
			}

			return true;
		};

		// Separator not found (End-of-string):
		while (current_position < str.length()) // && (current_position != std::string_view::npos)
		{
			std::size_t find_result = std::string_view::npos;

			const std::string_view* separator = nullptr;

			for (const auto& attempted_separator : separators)
			{
				const auto find_attempt = str.find(attempted_separator, current_position);

				if (find_attempt != std::string_view::npos)
				{
					if (find_attempt < find_result)
					{
						separator = &attempted_separator;

						find_result = find_attempt;
					}
				}
			}

			auto substr = std::string_view {};

			is_last_symbol = (find_result == std::string_view::npos);

			if (is_last_symbol)
			{
				if (separator_required && !separator_found)
				{
					break;
				}

				substr = str.substr(current_position);
			}
			// Separator found as expected:
			else
			{
				substr = str.substr(current_position, (find_result - current_position));

				// Separator found with no content between, skip forward:
				if ((find_result - current_position) == 0)
				{
					//assert(separator);

					if (include_empty_substrings)
					{
						if (!execute_callback(substr))
						{
							break;
						}
					}

					if (separator)
					{
						current_position = (find_result + separator->length()); // +=
					}
					else
					{
						current_position++;
					}

					continue;
				}
				else
				{
					separator_found = true;
				}
			}

			const auto initial_substr_length = substr.length();

			if (initial_substr_length == 0) // substr.empty()
			{
				if (!separator)
				{
					break;
				}
			}
			else
			{
				substr = trim(substr, trim_values);

				const auto trimmed_substr_length = substr.length();

				if (!execute_callback(substr))
				{
					break;
				}

				is_first_symbol = false;

				// Additional check added due to `callback`'s ability to change the size of `substr`:
				if ((!substr.empty()) && (substr.length() != trimmed_substr_length))
				{
					assert(substr.data() >= str.data());
					assert(substr.data() < (str.data() + str.length()));

					const auto substr_end_point = (substr.data() + substr.length());

					current_position = static_cast<std::size_t>(substr_end_point - str.data());

					continue;
				}
			}

			current_position += initial_substr_length;

			if (separator)
			{
				current_position += separator->length();
			}
		}

		return separator_found;
	}

	template <typename Callback>
	bool split
	(
		std::string_view str,
		std::string_view separator,
		Callback&& callback,
		bool separator_required=false,
		std::string_view trim_values=util::whitespace_symbols,
		bool include_empty_substrings=false
	)
	{
		return split
		(
			str,
			std::array { separator },
			std::forward<Callback>(callback),
			separator_required,
			trim_values,
			include_empty_substrings
		);
	}

	template <std::size_t expected_count, typename ArrayType=std::array<std::string_view, expected_count>>
	std::optional<std::tuple<ArrayType, std::size_t>> split_from_ex
	(
		std::string_view str,
		std::string_view separator,
		std::size_t min_allowed=expected_count,
		std::string_view trim_values=util::whitespace_symbols,
		bool include_empty_substrings=false
	)
	{
		auto out = ArrayType {};
		auto count = std::size_t {};

		split
		(
			str, separator,
			
			[&out, &count](std::string_view substr)
			{
				if (count >= out.size())
				{
					return;
				}

				out[count++] = substr;
			},

			(min_allowed > 1),

			trim_values,

			include_empty_substrings
		);

		if (count < min_allowed)
		{
			return std::nullopt;
		}

		return {{ std::move(out), count }};
	}

	template <std::size_t expected_count, typename ArrayType=std::array<std::string_view, expected_count>>
	std::optional<ArrayType> split_from
	(
		std::string_view str,
		std::string_view separator,
		std::size_t min_allowed=expected_count,
		std::string_view trim_values=util::whitespace_symbols,
		bool include_empty_substrings=false
	)
	{
		if (auto result = split_from_ex<expected_count, ArrayType>(str, separator, min_allowed, trim_values, include_empty_substrings))
		{
			return { std::move(std::get<0>(*result)) };
		}

		return std::nullopt;
	}

	template <typename StrArrayType>
	std::size_t count_valid_strings(const StrArrayType& str_array)
	{
		std::size_t count = 0;

		for (const auto& str_entry : str_array)
		{
			count += static_cast<std::size_t>(!str_entry.empty());
		}

		return count;
	}

	template <typename StrArrayType>
	std::size_t count_valid_strings(const std::optional<StrArrayType>& opt_str_array)
	{
		if (opt_str_array)
		{
			return count_valid_strings(*opt_str_array);
		}

		return 0;
	}

	template <bool short_circuit, typename FindFn>
	std::size_t find_singular(std::string_view str, std::string_view symbol, FindFn&& find_fn, std::size_t offset=0)
	{
		constexpr bool perform_look_ahead_on_early_symbol = true;

		auto position = offset;

		std::size_t cumulative_result = std::string_view::npos;

		std::size_t prev_result = std::string_view::npos;

		while (position < str.length())
		{
			auto result = find_fn(str, symbol, position);

			if (result == std::string_view::npos)
			{
				break;
			}

			const auto is_new_result_chain = ((prev_result == std::string_view::npos) || (result != (prev_result + symbol.length())));

			if (is_new_result_chain || (result <= symbol.length()))
			{
				const auto next_symbol_begin = (result + symbol.length());

				bool match_found = true;

				if constexpr (perform_look_ahead_on_early_symbol)
				{
					if (next_symbol_begin < str.length())
					{
						const auto next_symbol_area = str.substr(next_symbol_begin, symbol.length());

						match_found = (next_symbol_area != symbol);
					}
				}

				if (match_found)
				{
					if constexpr (short_circuit)
					{
						return result;
					}
					else
					{
						cumulative_result = result;
					}
				}
				else
				{
					// Avoid re-checking the next symbol and this symbol.
					result += symbol.length();
				}
			}
			else
			{
				const auto prev_symbol_area = str.substr((result - symbol.length()), symbol.length());

				if (prev_symbol_area != symbol)
				{
					// Immediate previous symbol not found.
					if constexpr (short_circuit)
					{
						return result;
					}
					else
					{
						cumulative_result = result;
					}
				}
			}

			prev_result = result;

			position = (result + symbol.length());
		}

		return cumulative_result;
	}

	std::size_t find_singular(std::string_view str, std::string_view symbol, std::size_t offset=0);
	std::size_t find_last_singular(std::string_view str, std::string_view symbol, std::size_t offset=0);

	// TODO: Move this to a different source file.
	std::string_view to_string_view(const aiString& str);

	std::string camel_to_snake_case(std::string_view camel_case);
	std::string snake_to_camel_case(std::string_view snake_case, bool leading_uppercase=true);

	std::string reverse_from_separator(std::string_view str, std::string_view separator, bool separator_required=false, std::string_view trim_values={});
}