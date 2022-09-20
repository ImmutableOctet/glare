#include "string.hpp"

#include <algorithm>

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
}