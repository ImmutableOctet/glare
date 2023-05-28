#pragma once

#include "types.hpp"
#include "common.hpp"

namespace math
{
	template <typename FromType, typename ToType, typename DeltaType=float>
	auto lerp(const FromType& from, const ToType& to, const DeltaType& delta)
	{
		return (from + ((to - from) * delta));
	}

	template <typename FromType, typename ToType, typename DeltaType=float>
	auto nlerp(const FromType& from, const ToType& to, const DeltaType& speed)
	{
		return normalize(lerp(from, to, speed));

		//return (from + (normalize(to - from) * speed));
	}

	// NOTE:
	// If `v0` and `v1` are already normalized, you can safely call
	// `slerp_unnormalized` instead for a minor performance improvement.
	Quaternion slerp(const Quaternion& v0, const Quaternion& v1, float t);

	// Same as `slerp`, but skips the normalization step.
	Quaternion slerp_unnormalized(Quaternion v0, Quaternion v1, float t);

	float nlerp_radians(const Vector& origin, const Vector& destination, float speed);
	float nlerp_radians(float origin, float destination, float speed);

	float nlerp_degrees(const Vector& origin, const Vector& destination, float speed);
	float nlerp_degrees(float origin, float destination, float speed);
}