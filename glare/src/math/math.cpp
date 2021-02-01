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

	Matrix to_matrix(const btTransform& t)
	{
		Matrix m;

		t.getOpenGLMatrix(glm::value_ptr(m));

		return m;
	}
}