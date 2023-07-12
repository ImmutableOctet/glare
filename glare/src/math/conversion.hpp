#pragma once

#include "types.hpp"
#include "constants.hpp"
#include "common.hpp"

#include <type_traits>

namespace math
{
	template <typename T>
	inline constexpr T degrees(T r)
	{
		return (r * (T(180) / _Pi<T>()));
	}

	inline constexpr Vector degrees(const Vector& r)
	{
		return { degrees(r.x), degrees(r.y), degrees(r.z) };
	}

	template <typename T>
	inline constexpr T radians(T d)
	{
		return (d * (_Pi<T>() / T(180)));
	}

	inline constexpr Vector radians(const Vector& d)
	{
		return { radians(d.x), radians(d.y), radians(d.z) };
	}

	// This is here primarily as a placeholder overload, in case
	// a templated function needs to convert between vector types.
	inline Vector3D to_vector(const Vector3D& v)
	{
		return v;
	}

	inline RotationMatrix to_rotation_matrix(const Quaternion& q)
	{
		return glm::toMat3(q);
	}

	inline Quaternion to_quaternion(const RotationMatrix& r)
	{
		return glm::quat_cast(r);
	}

	inline Quaternion to_quaternion(const Matrix& m)
	{
		return glm::quat_cast(m);
	}

	inline Matrix4x4 quat_to_mat4(const Quaternion& q)
	{
		return glm::mat4_cast(q);
	}

	inline Matrix3x3 quat_to_mat3(const Quaternion& q)
	{
		return glm::mat3_cast(q);
	}

	template <typename MatrixType>
	inline MatrixType quaternion_to_matrix(const Quaternion& q)
	{
		if constexpr (std::is_same_v<std::decay_t<MatrixType>, Matrix4x4>)
		{
			return quat_to_mat4(q);
		}
		else if constexpr (std::is_same_v<std::decay_t<MatrixType>, Matrix3x3>)
		{
			return quat_to_mat3(q);
		}
		else
		{
			return identity<MatrixType>(); // {};
		}
	}

	math::vec2f to_normalized_device_coordinates_ex(const math::vec2f& half_display_size, const math::vec2f& position);
	math::vec2f to_normalized_device_coordinates(const math::vec2f& display_size, const math::vec2f& position);

	math::vec2f from_normalized_device_coordinates_ex(const math::vec2f& half_display_size, const math::vec2f& normalized_position);
	math::vec2f from_normalized_device_coordinates(const math::vec2f& display_size, const math::vec2f& normalized_position);

	math::vec2f normalized_device_coordinates_to_screen_space(const math::vec2f& normalized_position);
	math::vec2f normalized_device_coordinates_to_screen_space(const math::vec2f& display_size, const math::vec2f& normalized_position);
}