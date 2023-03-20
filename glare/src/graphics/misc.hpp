#pragma once

#include <string_view>

//#include <string>

inline bool operator<(std::string_view a, const std::string& b) noexcept
{
	return (a < b);
}

/*
inline bool operator<(const std::string& a, std::string_view b) noexcept
{
	return (a < b);
}
*/