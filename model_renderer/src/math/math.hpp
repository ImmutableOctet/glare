#pragma once

#define GLM_FORCE_CTOR_INIT

#include <types.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace math
{
	// Types:
	using Vector2D = glm::vec2; using vec2f = Vector2D; using vec2 = vec2f;
	using Vector3D = glm::vec3; using vec3f = Vector3D; using vec3 = vec3f;
	using Vector4D = glm::vec4; using vec4f = Vector4D; using vec4 = vec4f;

	using Matrix2x2 = glm::mat2; using mat2f = Matrix2x2; using mat2 = mat2f;
	using Matrix3x3 = glm::mat3; using mat3f = Matrix3x3; using mat3 = mat3f;
	using Matrix4x4 = glm::mat4; using mat4f = Matrix4x4; using mat4 = mat4f;

	using AffineMatrix4 = glm::mat4x4; using affine_mat4 = AffineMatrix4; // glm::mat3x4

	using Vector = Vector3D;
	using Matrix = AffineMatrix4;
	using RotationMatrix = Matrix3x3;

	template <typename T>
	inline T sq(T x)
	{
		return (x * x);
	}

	// Retrieves the translation vector from a 4x4 or 3x4 (affine) matrix:
	template <typename mat4_type>
	inline Vector3D get_translation(const mat4_type& m)
	{
		const auto& translation = m[3];

		return { translation[0], translation[1], translation[2] };
	}

	template <typename matrix_type>
	inline constexpr matrix_type identity()
	{
		return matrix_type(1.0f); // glm::identity<matrix_type>();
	}

	template <typename quat_type>
	inline RotationMatrix to_rotation_matrix(const quat_type& q)
	{
		return glm::toMat3(q);
	}

	inline Vector3D operator*(const Matrix4x4& m, const Vector3D& v)
	{
		auto v4d = Vector4D(v, 0.0f);
		auto mv = (m * v4d);

		return Vector3D(mv);
	}

	float get_vector_pitch(const Vector& v);
	float get_vector_yaw(const Vector& v);

	float get_matrix_pitch(const RotationMatrix& m);
	float get_matrix_yaw(const RotationMatrix& m);
	float get_matrix_roll(const RotationMatrix& m);

	Vector get_rotation(const RotationMatrix& m);

	RotationMatrix rotation_pitch(float angle);
	RotationMatrix rotation_yaw(float angle);
	RotationMatrix rotation_roll(float angle);

	RotationMatrix rotation_from_vector(const Vector& rv);
}