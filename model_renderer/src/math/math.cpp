#include "math.hpp"

#include <cmath>

//#include <assimp/vector3.h>

namespace math
{
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
}