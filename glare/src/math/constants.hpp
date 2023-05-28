#pragma once

namespace math
{
	template <typename T>
	inline constexpr T _Pi()
	{
		return T(3.141592653589793); // M_PI
	}

	template <typename T>
	inline constexpr T _Tau()
	{
		return (static_cast<T>(2) * _Pi<T>());
	}

	constexpr auto Pi = _Pi<float>();
	constexpr auto Tau = _Tau<float>();
}