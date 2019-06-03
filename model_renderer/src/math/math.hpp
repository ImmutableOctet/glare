#pragma once

#include <types.hpp>
#include <glm/glm.hpp>

namespace math
{
	using Vector2D = glm::vec2; using vec2f = Vector2D; using vec2 = vec2f;
	using Vector3D = glm::vec3; using vec3f = Vector3D; using vec3 = vec3f;
	using Vector4D = glm::vec4; using vec4f = Vector4D; using vec4 = vec4f;

	using Matrix2x2 = glm::mat2; using mat2f = Matrix2x2; using mat2 = mat2f;
	using Matrix3x3 = glm::mat3; using mat3f = Matrix3x3; using mat3 = mat3f;
	using Matrix4x4 = glm::mat4; using mat4f = Matrix4x4; using mat4 = mat4f;

	//using Vector = Vector3D;
	//using Matrix = Matrix3x3;
}