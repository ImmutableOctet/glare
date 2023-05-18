#pragma once

#include "types.hpp"

#include "aabb.hpp"
#include "conversion.hpp"
#include "operators.hpp"

#include <cmath>

namespace math
{
	// Imported GLM functions:
	using glm::dot;
	using glm::cross;
	using glm::normalize;
	using glm::length;
	using glm::inverse;
	using glm::transpose;
	using glm::translate;
	using glm::rotate;
	using glm::scale;

	// Standard math functions:
	using std::abs;

	// Returns 1 for values greater than zero,
	// -1 for values less than zero,
	// and 0 for exactly 0.
	template <typename T, typename ResultType=int>
	inline ResultType sign(T value)
	{
		constexpr auto zero = T(0);

		return ResultType((zero < value) - (value < zero));
	}

	// Returns the `sign` from the result of: (x-y)
	//
	// If `x` is less than `y` this returns -1,
	// if `x` is greater than `y` this returns 1,
	// if `x` and `y` are equal this returns 0.
	template <typename T, typename ResultType=int>
	inline ResultType sign(const T& x, const T& y)
	{
		return sign<T, ResultType>((x - y));
	}

	// Forces `value` to be positive.
	template <typename T>
	inline T positive(const T& value)
	{
		return (value * sign(value));
	}

	// Forces `value` to be negative.
	template <typename T>
	inline T negative(const T& value)
	{
		return -positive(value);
	}

	// Inverts the sign of `value`.
	// (Equivalent to unary `-` operator)
	template <typename T>
	inline T negate(const T& value)
	{
		return -value;
	}

	template <typename T>
	inline T sq(const T& x)
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
		const auto i_l = glm::length(m[0]);
		const auto j_l = glm::length(m[1]);
		const auto k_l = glm::length(m[2]);

		return { i_l, j_l, k_l };
	}

	template <typename matrix_type>
	inline constexpr matrix_type identity()
	{
		return matrix_type(1.0f); // glm::identity<matrix_type>();
	}

	// Same as calling `identity<Matrix>()`.
	inline constexpr Matrix identity_matrix()
	{
		return identity<Matrix>();
	}

	inline Vector3D operator*(const Matrix4x4& m, const Vector3D& v)
	{
		const auto v4d = Vector4D(v, 0.0f);
		const auto mv = (m * v4d);

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

	Quaternion rotation_pitch_q(float angle);
	Quaternion rotation_yaw_q(float angle);
	Quaternion rotation_roll_q(float angle);

	// Aligned basis from direction vector `rv`.
	// See also: `rotation_from_orthogonal` and `quaternion_from_orthogonal`.
	RotationMatrix rotation_from_vector(const Vector& rv);

	Quaternion rotation_from_vector_q(const Vector& rv);

	Quaternion quaternion_from_orthogonal(const Vector& a, const Vector& b, const Vector& c);
	Quaternion quaternion_from_orthogonal(const Vector& a, const Vector& b); // Automatically computes `c`.
	Quaternion quaternion_from_orthogonal(const OrthogonalVectors& ortho_vectors);

	RotationMatrix rotation_from_orthogonal(const Vector& a, const Vector& b, const Vector& c);
	RotationMatrix rotation_from_orthogonal(const Vector& a, const Vector& b); // Automatically computes `c`.
	RotationMatrix rotation_from_orthogonal(const OrthogonalVectors& ortho_vectors);

	Vector abs(const Vector& v);

	// Converts a 2D direction to an euler angle.
	float direction_to_angle(const Vector2D& dir);

	// Similar to `direction_to_angle`, but rotated 90 degrees.
	// This is useful for analog gamepad input.
	float direction_to_angle_90_degrees(const Vector2D& dir);

	// Computes the 'yaw' angle of a 3D direction vector.
	float direction_to_yaw(const Vector& dir); // Vector3D

	template <typename T>
	inline T clamp(const T& value, const T& min_value, const T& max_value)
	{
		if (value <= min_value)
		{
			return min_value;
		}

		if (value >= max_value)
		{
			return max_value;
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
	inline auto lerp(T_Value&& value, T_Value&& dest, T_Delta&& delta)
	{
		return (value + ((dest - value) * delta));
	}

	inline Vector nlerp(const Vector& a, const Vector& b, float speed)
	{
		return glm::normalize(lerp(a, b, speed));

		//return (a + (glm::normalize(b - a) * speed));
	}

	float nlerp_radians(const Vector& origin, const Vector& destination, float speed);

	inline float nlerp_degrees(const Vector& origin, const Vector& destination, float speed)
	{
		return degrees(nlerp_radians(origin, destination, speed));
	}

	float nlerp_radians(float origin, float destination, float speed);

	inline float nlerp_degrees(float origin, float destination, float speed)
	{
		return degrees(nlerp_radians(degrees(origin), degrees(destination), speed));
	}

	// NOTE:
	// If `v0` and `v1` are already normalized, you can safely call
	// `slerp_unnormalized` instead for a minor performance improvement.
	Quaternion slerp(const Quaternion& v0, const Quaternion& v1, float t);

	// Same as `slerp`, but skips the normalization step.
	Quaternion slerp_unnormalized(Quaternion v0, Quaternion v1, float t);

	// Utility function; provides the cross-product (third direction-vector) of `normal` and `forward`.
	// See also: `get_surface_slope`.
	math::Vector get_surface_forward
	(
		const math::Vector& normal,
		const math::Vector& forward={0.0f, 0.0f, -1.0f}
	);

	// Returns the slope of the surface (`normal`)
	// from the approach angle (`angle`).
	float get_surface_slope
	(
		const math::Vector& normal,
		const math::Vector& angle,
		const math::Vector& forward={0.0f, 0.0f, -1.0f}
	);
}