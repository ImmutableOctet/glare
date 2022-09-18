#pragma once

#include "types.hpp"

namespace math
{
	template <typename T>
	constexpr T _Pi()
	{
		return T(3.141592653589793); // M_PI
	}

	constexpr auto Pi = _Pi<float>();

	template <typename T>
	inline constexpr T degrees(T r)
	{
		return (r * (T(180) / _Pi<T>()));
	}

	inline constexpr Vector degrees(const Vector& r)
	{
		return { degrees(r.x), degrees(r.y), degrees(r.z) };
	}

	template <typename T>
	inline constexpr T radians(T d)
	{
		return (d * (_Pi<T>() / T(180)));
	}

	inline constexpr Vector radians(const Vector& d)
	{
		return { radians(d.x), radians(d.y), radians(d.z) };
	}
}