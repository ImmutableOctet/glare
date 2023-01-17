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

    std::string reverse_from_separator(std::string_view str, std::string_view separator, std::string_view trim_values)
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

            trim_values
        );

        return out;
    }
}