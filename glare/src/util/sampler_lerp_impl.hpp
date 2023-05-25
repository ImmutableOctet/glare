#pragma once

#include <math/math.hpp>
#include <math/types.hpp>

#include <utility>

namespace util
{
	namespace impl
	{
		template <bool normalize=false>
		struct Sampler_BasicLerp
		{
			auto operator()(const auto& from_value, const auto& to_value, const auto interpolation_ratio) const
			{
				if constexpr (normalize)
				{
					return math::nlerp(from_value, to_value, interpolation_ratio);
				}
				else
				{
					return math::lerp(from_value, to_value, interpolation_ratio);
				}
			}
		};

		template <bool normalize=true> // false
		struct Sampler_Slerp
		{
			auto operator()(const auto& from_value, const auto& to_value, const auto interpolation_ratio) const
			{
				if constexpr (normalize)
				{
					return math::slerp(from_value, to_value, interpolation_ratio);
				}
				else
				{
					return math::slerp_unnormalized(from_value, to_value, interpolation_ratio);
				}
			}
		};
	}
}