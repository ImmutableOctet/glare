#include "math.hpp"
#include "bullet.hpp"

#include <cmath>

#include <glm/gtc/type_ptr.hpp>

//#include <assimp/vector3.h>

namespace math
{
	float get_vector_pitch(const Vector& v)
	{
		return -std::atan2(v.y, std::sqrt(sq(v.x) + sq(v.z)));
	}

	float get_vector_yaw(const Vector& v)
	{
		return -std::atan2(v.x, v.z);
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

		return std::atan2(i.y, j.y);
	}

	Vector get_rotation(const RotationMatrix& m)
	{
		return { get_matrix_pitch(m), get_matrix_yaw(m), get_matrix_roll(m) };
	}

	RotationMatrix rotation_pitch(float angle)
	{
		auto sin = std::sin(angle);
		auto cos = std::cos(angle);

		return { {1.0f, 0.0f, 0.0f}, {0.0f, cos, sin}, {0.0f, -sin, cos} };
	}

	RotationMatrix rotation_yaw(float angle)
	{
		auto sin = std::sin(angle);
		auto cos = std::cos(angle);

		return { {cos, 0, sin}, {0.0f, 1.0f, 0.0f}, {-sin, 0.0f, cos} };
	}

	RotationMatrix rotation_roll(float angle)
	{
		auto sin = std::sin(angle);
		auto cos = std::cos(angle);

		return { {cos, sin, 0.0f}, {-sin, cos, 0.0f}, {0.0f, 0.0f, 1.0f} };
	}

	RotationMatrix rotation_from_vector(const Vector& rv)
	{
		return (rotation_yaw(rv.y) * rotation_pitch(rv.x) * rotation_roll(rv.z));
	}

	Quaternion quaternion_from_orthogonal(const Vector& a, const Vector& b, const Vector& c)
	{
		if (auto T = (a.x + b.y + c.z); (T > 0))
		{
			auto s = (std::sqrt(T + 1) * 2.0f);

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
			auto s = (std::sqrt(1 + a.x - b.y - c.z) * 2);

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
			auto s = (std::sqrt(1 + b.y - a.x - c.z) * 2.0f);

			return Quaternion
			(
				((b.z - c.y) / s), // W
				((b.x + a.y) / s), // X
				(s * 0.25f),       // Y
				((c.y + b.z) / s)  // Z
			);
		}

		auto s = (std::sqrt(1 + c.z - a.x - b.y) * 2.0f);

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

		auto angle = std::acos(glm::dot(b, a) / (glm::length(b) * glm::length(a)));

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
	
	Vector abs(const Vector& v)
	{
		return { std::abs(v.x), std::abs(v.y), std::abs(v.z) };
	}

	float direction_to_angle(const Vector2D& dir)
	{
		//return std::atan2(std::cos(dir.y), std::sin(dir.x));
		//return std::atan2(dir.y, dir.x);
		return std::atan2(dir.x, dir.y);
	}

	float direction_to_angle_90_degrees(const Vector2D& dir)
	{
		return std::atan2(dir.x, -dir.y);
	}

	float direction_to_yaw(const Vector& dir)
	{
		return std::atan2(dir.x, -dir.z);
	}

	float nlerp_radians(const Vector& origin, const Vector& destination, float speed)
	{
		auto lerp_dir = nlerp(origin, destination, speed);

		return direction_to_yaw(lerp_dir); // std::atan2(lerp_dir.x, -lerp_dir.z);
	}

	float nlerp_radians(float origin, float destination, float speed)
	{
		auto dir = Vector2D(std::cos(origin), std::sin(origin));
		auto dest = Vector2D(std::cos(destination), std::sin(destination));

		dir = lerp(dir, dest, speed);

		return std::atan2(dir.y, dir.x);
	}

	Quaternion slerp(const Quaternion& v0, const Quaternion& v1, float t)
	{
		return slerp_unnormalized(glm::normalize(v0), glm::normalize(v1), t);
	}

	Quaternion slerp_unnormalized(Quaternion v0, Quaternion v1, float t)
	{
		auto dot = glm::dot(v0, v1);

		constexpr auto DOT_THRESHOLD = 0.9995f;

		if (std::abs(dot) > DOT_THRESHOLD)
		{
			auto result = v0;

			result += ((v1 - v0) * t);

			return glm::normalize(result);
		}

		if (dot < 0.0f)
		{
			v1 = -v1;
			dot = -dot;
		}

		dot = clamp(dot, -1.0f, 1.0f);

		auto theta_0 = std::acos(dot);
		auto theta   = (theta_0 * t);

		auto v2 = glm::normalize(v1 - v0 * dot);

		return (v0 * std::cos(theta) + v2 * std::sin(theta));
	}

	math::Vector get_surface_forward(const math::Vector& normal, const math::Vector& forward)
	{
		return cross(normal, forward);
	}

	float get_surface_slope(const math::Vector& normal, const math::Vector& angle, const math::Vector& forward)
	{
		auto adjacent = get_surface_forward(normal, forward);

		return glm::dot(angle, adjacent);
	}
}