#include "rotation.hpp"

#include "common.hpp"

namespace math
{
	float get_vector_pitch(const Vector& v)
	{
		return -atan2(v.y, sqrt(sq(v.x) + sq(v.z)));
	}

	float get_vector_yaw(const Vector& v)
	{
		return -atan2(v.x, v.z);
	}

	float get_matrix_pitch(const RotationMatrix& m)
	{
		auto& k = m[2];

		return get_vector_pitch(k);
	}

	float get_matrix_yaw(const RotationMatrix& m)
	{
		auto& k = m[2];

		return get_vector_yaw(k);
	}

	float get_matrix_roll(const RotationMatrix& m)
	{
		auto& i = m[0];
		auto& j = m[1];

		return atan2(i.y, j.y);
	}

	Vector get_rotation(const RotationMatrix& m)
	{
		return { get_matrix_pitch(m), get_matrix_yaw(m), get_matrix_roll(m) };
	}

	RotationMatrix rotation_pitch(float angle)
	{
		const auto angle_sin = sin(angle);
		const auto angle_cos = cos(angle);

		return
		{
			{1.0f, 0.0f, 0.0f},
			{0.0f, angle_cos, angle_sin},
			{0.0f, -angle_sin, angle_cos}
		};
	}

	RotationMatrix rotation_yaw(float angle)
	{
		const auto angle_sin = sin(angle);
		const auto angle_cos = cos(angle);

		return
		{
			{angle_cos, 0, angle_sin},
			{0.0f, 1.0f, 0.0f},
			{-angle_sin, 0.0f, angle_cos}
		};
	}

	RotationMatrix rotation_roll(float angle)
	{
		const auto angle_sin = sin(angle);
		const auto angle_cos = cos(angle);

		return
		{
			{angle_cos, angle_sin, 0.0f},
			{-angle_sin, angle_cos, 0.0f},
			{0.0f, 0.0f, 1.0f}
		};
	}

	Quaternion rotation_pitch_q(float angle)
	{
		return Quaternion { rotation_pitch(angle) };

		//return glm::rotate(Quaternion{}, angle, { 1.0f, 0.0f, 0.0f });
	}

	Quaternion rotation_yaw_q(float angle)
	{
		return Quaternion { rotation_yaw(angle) };

		//return glm::rotate(Quaternion {}, angle, { 0.0f, 1.0f, 0.0f });
	}

	Quaternion rotation_roll_q(float angle)
	{
		return Quaternion { rotation_roll(angle) };

		//return glm::rotate(Quaternion {}, angle, { 0.0f, 0.0f, 1.0f });
	}

	RotationMatrix rotation_from_vector(const Vector& rv)
	{
		return (rotation_yaw(rv.y) * rotation_pitch(rv.x) * rotation_roll(rv.z));
	}

	Quaternion rotation_from_vector_q(const Vector& rv)
	{
		// TODO: Change to a more optimal implementation.
		// (i.e. without matrix conversion)
		return Quaternion { rotation_from_vector(rv) };

		// Alternative implementation (untested 'optimal' version):
		//return (rotation_yaw_q(rv.y) * rotation_pitch_q(rv.x) * rotation_roll_q(rv.z));
	}

	Quaternion quaternion_from_orthogonal(const Vector& a, const Vector& b, const Vector& c)
	{
		if (auto T = (a.x + b.y + c.z); (T > 0))
		{
			auto s = (sqrt(T + 1) * 2.0f);

			return Quaternion
			(
				(s * 0.25f),       // W
				((c.y - b.z) / s), // X
				((a.z - c.x) / s), // Y
				((b.x - a.y) / s)  // Z
			);
		}

		if ((a.x > b.y) && (a.x > c.z))
		{
			auto s = (sqrt(1 + a.x - b.y - c.z) * 2);

			return Quaternion
			(
				((c.y - b.z) / s), // W
				(s * 0.25f),       // X
				((b.x + a.y) / s), // Y
				((a.z + c.x) / s)  // Z
			);
		}

		if (b.y > c.z)
		{
			auto s = (sqrt(1 + b.y - a.x - c.z) * 2.0f);

			return Quaternion
			(
				((b.z - c.y) / s), // W
				((b.x + a.y) / s), // X
				(s * 0.25f),       // Y
				((c.y + b.z) / s)  // Z
			);
		}

		auto s = (sqrt(1 + c.z - a.x - b.y) * 2.0f);

		return Quaternion
		(
			((b.x - a.y) / s), // W
			((a.z + c.x) / s), // X
			((c.y + b.z) / s), // Y
			(s * 0.25f)        // Z
		);
	}

	Quaternion quaternion_from_orthogonal(const Vector& a, const Vector& b)
	{
		return quaternion_from_orthogonal(a, b, cross(a, b));
	}

	Quaternion quaternion_from_orthogonal(const OrthogonalVectors& ortho_vectors)
	{
		const auto& [a, b, c] = ortho_vectors;

		return quaternion_from_orthogonal(a, b, c);
	}

	RotationMatrix rotation_from_orthogonal(const Vector& a, const Vector& b, const Vector& c)
	{
		/*
		glm::mat4 rotation = identity_matrix();

		auto angle = acos(dot(b, a) / (length(b) * length(a)));

		glm::rotate(rotation, angle, c);

		return rotation;
		*/

		//return glm::lookAt(...);

		auto q = quaternion_from_orthogonal(a, b, c);

		return to_rotation_matrix(q);
	}

	RotationMatrix rotation_from_orthogonal(const Vector& a, const Vector& b)
	{
		return rotation_from_orthogonal(a, b, cross(a, b));
	}

	RotationMatrix rotation_from_orthogonal(const OrthogonalVectors& ortho_vectors)
	{
		const auto& [a, b, c] = ortho_vectors;

		return rotation_from_orthogonal(a, b, c);
	}

	float direction_to_angle(const Vector2D& dir)
	{
		//return atan2(cos(dir.y), sin(dir.x));
		//return atan2(dir.y, dir.x);
		return atan2(dir.x, dir.y);
	}

	float direction_to_angle_90_degrees(const Vector2D& dir)
	{
		return atan2(dir.x, -dir.y);
	}

	float direction_to_yaw(const Vector& dir)
	{
		return atan2(dir.x, -dir.z);
	}
}