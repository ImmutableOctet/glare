#include "string.hpp"

#include <algorithm>

namespace util
{
    std::string lowercase(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });

        return str;
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
}