#include "string.hpp"

#include <algorithm>

namespace util
{
    std::string lowercase(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });

        return str;
    }
}