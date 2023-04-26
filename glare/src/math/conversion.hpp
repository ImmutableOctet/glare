#pragma once

#include "types.hpp"

namespace math
{
	template <typename T>
	inline constexpr T _Pi()
	{
		return T(3.141592653589793); // M_PI
	}

	constexpr auto Pi = _Pi<float>();

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
	
	// This is mainly here as a placeholder overload, in case
	// a templated function needs to convert between vector types.
	inline Vector3D to_vector(const Vector3D& v) { return v; }

	math::vec2f to_normalized_device_coordinates_ex(const math::vec2f& half_display_size, const math::vec2f& position);
	math::vec2f to_normalized_device_coordinates(const math::vec2f& display_size, const math::vec2f& position);

	math::vec2f from_normalized_device_coordinates_ex(const math::vec2f& half_display_size, const math::vec2f& normalized_position);
	math::vec2f from_normalized_device_coordinates(const math::vec2f& display_size, const math::vec2f& normalized_position);
}