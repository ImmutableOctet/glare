#pragma once

#define GLM_FORCE_CTOR_INIT

//#include "common.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
//#include <glm/gtc/quaternion.hpp>
#include <glm/mat3x4.hpp>

//#include <cstdint>
//#include <cmath>
#include <tuple>

namespace math
{
	using Vector2D = glm::vec2; using vec2f = Vector2D; using vec2 = vec2f;
	using Vector3D = glm::vec3; using vec3f = Vector3D; using vec3 = vec3f;
	using Vector4D = glm::vec4; using vec4f = Vector4D; using vec4 = vec4f;

	using vec3i = glm::ivec3;
	using vec2i = glm::ivec2;
	using vec4i = glm::ivec4;

	using Matrix2x2 = glm::mat2; using mat2f = Matrix2x2; using mat2 = mat2f;
	using Matrix3x3 = glm::mat3; using mat3f = Matrix3x3; using mat3 = mat3f;
	using Matrix4x4 = glm::mat4; using mat4f = Matrix4x4; using mat4 = mat4f;

	//using AffineMatrix4 = glm::mat4x4;
	using AffineMatrix4 = glm::mat3x4;
	using affine_mat4 = AffineMatrix4;

	using Vector = Vector3D;
	using Matrix = Matrix4x4; // AffineMatrix4;
	using RotationMatrix = Matrix3x3;

	using Quaternion = glm::quat;
	using Quat = Quaternion;

	using TransformVectors = std::tuple<Vector, Vector, Vector>; // Position, Rotation, Scale
	using OrthogonalVectors = std::tuple<Vector, Vector, Vector>; // Left, Up, Forward

	/*
	// Disabled for now:
	inline Vector3D operator*(const Matrix4x4& m, const Vector3D& v)
	{
		const auto v4d = Vector4D(v, 0.0f); // 1.0f
		const auto mv = (m * v4d);

		return Vector3D(mv);
	}
	*/
}