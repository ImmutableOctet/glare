#pragma once

#include "uniform_data.hpp"

#include <map>
//#include <unordered_map>

#include <string>
//#include <string_view>

namespace graphics
{
	using UniformMap = std::map<std::string, UniformData, std::less<>>; // std::string_view // unordered_map
}