#pragma once

#include "types.hpp"

// Output stream utility operators:

template <typename OutStream>
inline auto& operator<<(OutStream& os, const math::Vector& v)
{
	os << v.x << ", " << v.y << ", " << v.z;

	return os;
}

template <typename OutStream>
inline auto& operator<<(OutStream& os, const math::TransformVectors& v)
{
	os << "{" << std::get<0>(v) << "}, ";
	os << "{" << std::get<1>(v) << "}, ";
	os << "{" << std::get<2>(v) << "}";

	return os;
}