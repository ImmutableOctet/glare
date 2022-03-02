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

	Vector3D to_vector(const btVector3& v)
	{
		return { v.getX(), v.getY(), v.getZ() };
	}

	Vector3D to_vector(const aiVector3D& v)
	{
		return { v.x, v.y, v.z };
	}

	Matrix to_matrix(const btTransform& t)
	{
		Matrix m;

		t.getOpenGLMatrix(glm::value_ptr(m));

		return m;
	}

	Matrix to_matrix(const aiMatrix4x4& m)
	{
		Matrix out;
		
		// Fields a, b, c, and d from Assimp effectively need to be transposed/flipped for GLM:
		out[0][0] = m.a1; out[1][0] = m.a2; out[2][0] = m.a3; out[3][0] = m.a4;
		out[0][1] = m.b1; out[1][1] = m.b2; out[2][1] = m.b3; out[3][1] = m.b4;
		out[0][2] = m.c1; out[1][2] = m.c2; out[2][2] = m.c3; out[3][2] = m.c4;
		out[0][3] = m.d1; out[1][3] = m.d2; out[2][3] = m.d3; out[3][3] = m.d4;

		return out;
	}

	Quaternion to_quat(const aiQuaternion& q)
	{
		return Quaternion(q.w, q.x, q.y, q.z);
	}

	Quaternion to_quat_flipped(const aiQuaternion& q)
	{
		//return Quaternion(-q.w, q.z, q.y, q.x);
		//return Quaternion(q.w, q.x, -q.y, q.z);
		//return Quaternion(q.w, q.z, q.y, q.x);
		//return Quaternion(q.w, -q.z, q.y, -q.x);

		//return Quaternion(q.w, -q.x, q.y, -q.z);
		return Quaternion(q.w, -q.x, -q.y, -q.z);
	}
	
	Vector abs(Vector v)
	{
		return { std::abs(v.x), std::abs(v.y), std::abs(v.z) };
	}

	float direction_to_angle(const Vector2D& dir)
	{
		//return std::atan2(std::cos(dir.y), std::sin(dir.x));
		return std::atan2(dir.y, dir.x);
	}

	float direction_to_yaw(const Vector& dir)
	{
		return std::atan2(dir.x, -dir.z);
	}

	float nlerp_radians(Vector origin, Vector destination, float speed)
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

	Quaternion slerp(Quaternion v0, Quaternion v1, float t)
	{
		v0 = glm::normalize(v0);
		v1 = glm::normalize(v1);

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
}