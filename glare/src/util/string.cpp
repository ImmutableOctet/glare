#include "string.hpp"

#include <algorithm>
#include <cctype>

#include <assimp/types.h>

namespace util
{
    std::string lowercase(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });

        return str;
    }

    std::string lowercase(std::string_view str)
    {
        return lowercase(std::string(str));
    }

    std::string uppercase(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });

        return str;
    }

    std::string uppercase(std::string_view str)
    {
        return uppercase(std::string(str));
    }

    std::size_t replace(std::string& str, std::string_view from, std::string_view to, std::size_t offset, bool exhaustive)
    {
        auto occurrences = std::size_t {};

        auto position = offset;

        do
        {
            position = str.find(from, position);

            if (position == std::string::npos)
            {
                break;
            }

            str.replace(position, from.length(), to);

            occurrences++;
        } while (exhaustive);

        return occurrences;
    }

    std::string replace(std::string_view view_of_original, std::string_view from, std::string_view to, std::size_t offset, bool exhaustive)
    {
        auto output = std::string { view_of_original };

        replace(output, from, to, offset, exhaustive);

        return output;
    }

    std::smatch get_regex_groups(const std::string& s, const std::regex& re)
    {
        if (std::regex_match(s, re))
        {
            std::smatch m;
            std::regex_search(s, m, re);

            return m;
        }

        return {};
    }

    std::smatch parse_regex(const std::string& str, const std::string& regex_str)
    {
        return get_regex_groups(str, std::regex(regex_str));
    }

    std::string_view trim_beginning(std::string_view str, std::string_view trim_values)
    {
        if (str.empty())
        {
            return {};
        }

        if (trim_values.empty())
        {
            return str;
        }

        const auto content_start_index = str.find_first_not_of(trim_values);

        if (content_start_index == std::string_view::npos)
        {
            return str;
        }

        str.remove_prefix(content_start_index);

        return str;
    }

    std::string_view trim_ending(std::string_view str, std::string_view trim_values)
    {
        if (str.empty())
        {
            return {};
        }

        if (trim_values.empty())
        {
            return str;
        }

        const auto content_end_index = str.find_last_not_of(trim_values);

        if (content_end_index == std::string_view::npos)
        {
            return str;
        }

        str.remove_suffix((str.length() - content_end_index - 1));

        return str;
    }

    std::string_view trim(std::string_view str, std::string_view trim_values, bool empty_on_only_trim_values)
	{
        str = trim_ending(trim_beginning(str, trim_values), trim_values);

        if (empty_on_only_trim_values)
        {
            if (is_whitespace(str, trim_values))
            {
                return {};
            }
        }

        return str;
	}

    std::string_view trim(std::string_view str, bool empty_on_only_whitespace)
    {
        return trim(str, whitespace_symbols, empty_on_only_whitespace);
    }

    bool is_whitespace(std::string_view str, std::string_view whitespace_values)
    {
        if (str.empty())
        {
            return false;
        }

        if (whitespace_values.empty())
        {
            return false;
        }

        for (const auto chr : str)
        {
            if (!is_whitespace(chr, whitespace_values))
            {
                return false;
            }
        }

        return true;
    }

    bool is_whitespace(char symbol, std::string_view whitespace_values)
    {
        //return (whitespace_values.find(symbol) != std::string_view::npos);
        return whitespace_values.contains(symbol);
    }

    std::string_view remap_string_view(std::string_view old_basis, std::string_view new_basis, std::string_view old_view)
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

    std::string_view match_view(std::string_view str, const std::smatch& matches, std::size_t index, std::string_view trim_values)
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

	std::string& remove_quotes(std::string& str)
	{
		str.erase(0, 1);
		str.erase(str.length() - 1);

		return str;
	}

	std::string& remove_quotes_safe(std::string& str)
	{
		// NOTE: Length check performed inside `is_quoted`.
		if (!is_quoted(std::string_view(str)))
		{
			return str;
		}

		return remove_quotes(str);
	}

    std::string quote(std::string_view str)
    {
        //return format("\"{}\"", str);

        return "\"" + std::string(str) + "\"";
    }

    std::string_view to_string_view(const aiString& str)
    {
        return { str.C_Str(), str.length };
    }

    std::string camel_to_snake_case(std::string_view camel_case)
    {
        std::string out;

        if (camel_case.empty())
        {
            return out;
        }

        out.reserve(camel_case.size()); // length()

        out.push_back(std::tolower(camel_case[0]));

        //for (auto symbol : camel_case)
        for (std::size_t i = 1; i < camel_case.length(); i++)
        {
            auto symbol = camel_case[i];

            if (std::isupper(symbol))
            {
                out.push_back('_');
                out.push_back(std::tolower(symbol));

                continue;
            }

            out.push_back(symbol);
        }

        return out;
    }

    std::string snake_to_camel_case(std::string_view snake_case, bool leading_uppercase)
    {
        std::string out;

        if (snake_case.empty())
        {
            return out;
        }

        out.reserve(snake_case.size()); // length()

        if (leading_uppercase)
        {
            out.push_back(std::toupper(snake_case[0]));
        }
        else
        {
            out.push_back(std::tolower(snake_case[0]));
        }

        //for (auto symbol : camel_case)
        for (std::size_t i = 1; i < snake_case.length(); i++)
        {
            auto symbol = snake_case[i];

            if (symbol == '_')
            {
                if ((++i) >= snake_case.length())
                {
                    break;
                }

                // Retrieve next symbol as uppercase.
                symbol = std::toupper(snake_case[i]);
            }

            out.push_back(symbol);
        }

        return out;
    }

    std::string reverse_from_separator(std::string_view str, std::string_view separator, bool separator_required, std::string_view trim_values)
    {
        std::string out;

        out.reserve(str.size());

        auto insert = [&out](const auto& element)
        {
            const auto init_size = out.size();
            out.resize(init_size + element.size());

            std::copy(out.begin(), (out.begin() + init_size), out.begin() + element.size()); // std::move();
            std::copy(element.begin(), element.end(), out.begin());
        };

        split
        (
            str, separator,

            [&separator, &insert](std::string_view element, bool is_last_element=false)
            {
                insert(element);

                if (!is_last_element)
                {
                    insert(separator);
                }
            },

            separator_required,

            trim_values
        );

        return out;
    }

    bool is_quoted(std::string_view str, char quote_char)
	{
        //return simplified_is_quoted(str, quote_char);

		// TODO: Revisit how this function handles whitespace.
		//str = trim(str);

		if (str.length() < 2)
		{
			return false;
		}

		const auto first_quote = str.find(quote_char);

		if (first_quote != 0) // || (first_quote == std::string_view::npos)
		{
			return false;
		}

		// Iterative implementation:
		std::size_t offset = (first_quote + 1); // 1

		while (offset < str.length())
		{
			const auto second_quote = str.find(quote_char, offset);

			if (second_quote == std::string_view::npos)
			{
				break;
			}

			const auto prev_char = str[second_quote - 1];

			if (prev_char == '\\')
			{
				offset = (second_quote + 1);

				continue;
			}

			return (second_quote == (str.length() - 1));
		}

		// Non-iterative implementation (Has gaps/edge-cases):
        /*
		if (str.ends_with(quote_char))
		{
			const auto prev_char = str[str.length() - 2];

			if (prev_char == '\\')
			{
				return false;
			}

			return true;
		}
        */

		return false;
	}

    std::size_t find_singular(std::string_view str, std::string_view symbol, std::size_t offset)
	{
		return find_singular<true>
		(
			str, symbol,
			
			[](std::string_view str, std::string_view symbol, std::size_t position)
			{
				return str.find(symbol, position);
			},

			offset
		);
	}

	std::size_t find_last_singular(std::string_view str, std::string_view symbol, std::size_t offset)
	{
		return find_singular<false>
		(
			str, symbol,
			
			[](std::string_view str, std::string_view symbol, std::size_t position)
			{
				return str.find(symbol, position);
			},

			offset
		);
	}

    std::size_t find_unescaped(std::string_view str, std::string_view target_symbol, std::size_t offset, std::string_view escape_symbol)
	{
		const auto result = str.find(target_symbol, offset);

		if ((result != std::string_view::npos) && (result >= escape_symbol.length()))
		{
			const auto escape_area = std::string_view { (str.data() + result - escape_symbol.length()), escape_symbol.length() };

			if (escape_area == escape_symbol)
			{
				const auto updated_offset = (result + target_symbol.length());

				return find_unescaped(str, target_symbol, updated_offset, escape_symbol);
			}
		}

		return result;
	}
}