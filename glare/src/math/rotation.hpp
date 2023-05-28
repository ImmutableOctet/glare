#pragma once

#include "types.hpp"
#include "constants.hpp"
#include "conversion.hpp"
#include "lerp.hpp"

namespace math
{
	template <typename T>
	T wrap_angle(T angle_deg)
	{
		while (angle_deg < 0)
		{
			angle_deg += 360;
		}

		return angle_deg;
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

	// Converts a 2D direction to an euler angle.
	float direction_to_angle(const Vector2D& dir);

	// Similar to `direction_to_angle`, but rotated 90 degrees.
	// This is useful for analog gamepad input.
	float direction_to_angle_90_degrees(const Vector2D& dir);

	// Computes the 'yaw' angle of a 3D direction vector.
	float direction_to_yaw(const Vector& dir); // Vector3D
}