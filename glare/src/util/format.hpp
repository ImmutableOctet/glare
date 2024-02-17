#pragma once

#include <string_view>

//#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include "magic_enum.hpp"

#include <magic_enum_format.hpp>

// Standard alternative:
//#include <format>
//using fmt = std;

//namespace engine { using fmt::format; }

namespace util
{
	using fmt::format;
}