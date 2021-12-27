#pragma once

#define GLM_FORCE_CTOR_INIT

#include <types.hpp>

//#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
//#include <glm/gtc/quaternion.hpp>
#include <glm/mat3x4.hpp>

#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>
#include <assimp/Quaternion.h>

#include <tuple>

// Bullet types:
class btVector3;
class btTransform;

// Assimp types:
//class aiVector3D;

namespace math
{
	// Types:
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

	template <typename T>
	static constexpr T _Pi = static_cast<T>(3.141592653589793); // M_PI

	static constexpr auto Pi = _Pi<float>;

	template <typename T>
	inline constexpr T degrees(T r)
	{
		static constexpr T R_TO_DEG = (static_cast<T>(180) / _Pi<T>);

		return (r * R_TO_DEG);
	}

	inline constexpr Vector degrees(Vector r)
	{
		return { degrees(r.x), degrees(r.y), degrees(r.z) };
	}

	template <typename T>
	inline constexpr T radians(T d)
	{
		static constexpr T DEG_TO_R = (_Pi<T> / static_cast<T>(180));

		return (d * DEG_TO_R);
	}

	inline constexpr Vector radians(Vector d)
	{
		return { radians(d.x), radians(d.y), radians(d.z) };
	}

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

	template <typename mat_type>
	inline Vector3D get_scaling(const mat_type& m) // Matrix
	{
		auto i_l = glm::length(m[0]);
		auto j_l = glm::length(m[1]);
		auto k_l = glm::length(m[2]);

		return { i_l, j_l, k_l };
	}

	template <typename matrix_type>
	inline constexpr matrix_type identity()
	{
		return matrix_type(1.0f); // glm::identity<matrix_type>();
	}

	inline constexpr Matrix identity_matrix()
	{
		return identity<Matrix>();
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

	inline Vector3D to_vector(const Vector3D& v) { return v; }

	Vector3D to_vector(const btVector3& v);
	Vector3D to_vector(const aiVector3D& v);

	Matrix to_matrix(const btTransform& t);
	Matrix to_matrix(const aiMatrix4x4& m);

	Quaternion to_quat(const aiQuaternion& q); // to_quaternion(...)

	Vector abs(Vector v);

	float direction_to_angle(const Vector2D& dir);
	float direction_to_yaw(const Vector& dir); // Vector3D

	template <typename T>
	inline T clamp(T value, T min_value, T max_value)
	{
		if (value <= min_value)
		{
			return min_value;
		}

		if (value >= max_value)
		{
			return value;
		}

		return value;
	}

	template <typename T>
	inline T wrap_angle(T angle)
	{
		while (angle < 0)
		{
			angle += 360;
		}

		return angle;
	}

	template <typename T_Value, typename T_Delta>
	inline auto lerp(T_Value value, T_Value dest, T_Delta delta)
	{
		return (value + ((dest - value) * delta));
	}

	inline Vector nlerp(Vector a, Vector b, float speed)
	{
		return glm::normalize(lerp(a, b, speed));

		//return (a + (glm::normalize(b - a) * speed));
	}

	float nlerp_radians(Vector origin, Vector destination, float speed);

	inline float nlerp_degrees(Vector origin, Vector destination, float speed)
	{
		return degrees(nlerp_radians(origin, destination, speed));
	}

	float nlerp_radians(float origin, float destination, float speed);

	inline float nlerp_degrees(float origin, float destination, float speed)
	{
		return degrees(nlerp_radians(degrees(origin), degrees(destination), speed));
	}

	Quaternion slerp(Quaternion v0, Quaternion v1, float t);
}

namespace graphics
{
	using ColorRGB = math::vec3f;
	using ColorRGBA = math::vec4f;

	using Color = ColorRGB;
}

template <typename OutStream>
inline auto& operator<<(OutStream& os, const math::Vector& v)
{
	os << v.x << ", " << v.y << ", " << v.z;

	return os;
}

template <typename OutStream>
inline auto& operator<<(OutStream& os, const math::TransformVectors& v)
{
	os << "{" << std::get<0>(v) << "}, ";
	os << "{" << std::get<1>(v) << "}, ";
	os << "{" << std::get<2>(v) << "}";

	return os;
}