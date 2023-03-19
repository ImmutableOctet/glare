#pragma once

#include "array_types.hpp"

#include <math/types.hpp>

#include <variant>

namespace graphics
{
	using UniformData  = std::variant
	<
		bool, int, float, ContextHandle,
		math::Vector2D, math::Vector3D, math::Vector4D,
		math::Matrix2x2, math::Matrix3x3, math::Matrix4x4,
			
		VectorArray,
		MatrixArray,
		FloatArray
	>;
}