#include "lerp.hpp"

#include "rotation.hpp"

namespace math
{
	float nlerp_radians(const Vector& origin, const Vector& destination, float speed)
	{
		auto lerp_dir = nlerp(origin, destination, speed);

		return direction_to_yaw(lerp_dir); // atan2(lerp_dir.x, -lerp_dir.z);
	}

	float nlerp_radians(float origin, float destination, float speed)
	{
		const auto destination_direction = Vector2D(cos(destination), sin(destination));

		auto origin_direction = Vector2D(cos(origin), sin(origin));

		origin_direction = lerp(origin_direction, destination_direction, speed);

		return atan2(origin_direction.y, origin_direction.x);
	}

	float nlerp_degrees(const Vector& origin, const Vector& destination, float speed)
	{
		return degrees(nlerp_radians(origin, destination, speed));
	}

	float nlerp_degrees(float origin, float destination, float speed)
	{
		return degrees(nlerp_radians(degrees(origin), degrees(destination), speed));
	}

	Quaternion slerp(const Quaternion& v0, const Quaternion& v1, float t)
	{
		return slerp_unnormalized(normalize(v0), normalize(v1), t);
	}

	Quaternion slerp_unnormalized(Quaternion v0, Quaternion v1, float t)
	{
		constexpr auto DOT_THRESHOLD = 0.9995f;

		auto dot_product = dot(v0, v1);

		if (abs(dot_product) > DOT_THRESHOLD)
		{
			auto result = v0;

			result += ((v1 - v0) * t);

			return normalize(result);
		}

		if (dot_product < 0.0f)
		{
			v1 = -v1;

			dot_product = -dot_product;
		}

		dot_product = clamp(dot_product, -1.0f, 1.0f);

		const auto theta_0 = acos(dot_product);
		const auto theta   = (theta_0 * t);

		const auto v2 = normalize(v1 - v0 * dot_product);

		return (v0 * cos(theta) + v2 * sin(theta));
	}
}