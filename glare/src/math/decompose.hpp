#pragma once

#include "types.hpp"
#include "common.hpp"

namespace math
{
	// Retrieves the translation vector from a 4x4 or 3x4 (affine) matrix:
	template <typename mat4_type>
	inline Vector3D get_translation(const mat4_type& m)
	{
		const auto& translation = m[3];

		return { translation[0], translation[1], translation[2] };
	}

	template <typename mat_type>
	inline Vector3D get_scaling(const mat_type& m) // Matrix
	{
		const auto i_l = length(m[0]);
		const auto j_l = length(m[1]);
		const auto k_l = length(m[2]);

		return { i_l, j_l, k_l };
	}
}