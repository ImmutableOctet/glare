#pragma once

#include "reflect_decl.hpp"

#include <math/types.hpp>

#include <util/common_macros.hpp>

#define GLARE_IMPL_CORE_MATH_TYPES(DECL_FN) \
	DECL_FN(math::Vector2D)                                 \
	DECL_FN(math::Vector3D)                                 \
	DECL_FN(math::Vector4D)                                 \
	DECL_FN(math::vec2i)                                    \
	DECL_FN(math::Matrix4x4)                                \
	DECL_FN(math::Matrix3x3)                                \
	DECL_FN(math::Quaternion)                               \
	DECL_FN(math::TransformVectors)

#define GLARE_IMPL_DECLARE_CORE_MATH_REFLECTION() \
	GLARE_IMPL_CORE_MATH_TYPES(GLARE_IMPL_DECLARE_REFLECTED_TYPE)	

#define GLARE_IMPL_REFLECTED_MATH_TYPES \
	GLARE_IMPL_CORE_MATH_TYPES(GLARE_IMPL_COMMA_SEPARATE) \
	engine::Math

#define GLARE_IMPL_REFLECTED_MATH_TYPES_TUPLE \
	std::tuple<GLARE_IMPL_REFLECTED_MATH_TYPES>

namespace engine
{
	// Reserved type, used primarily for reflection.
	struct Math;

	template <> void reflect<Math>();

	GLARE_IMPL_DECLARE_CORE_MATH_REFLECTION();
}