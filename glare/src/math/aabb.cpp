#include "aabb.hpp"

#include "common.hpp"

namespace math
{
	Vector AABB::dim_lengths() const
	{
		return (max - min);
	}

	float AABB::length() const
	{
		return math::length(dim_lengths());
	}

	float AABB::average_length() const
	{
		auto len = dim_lengths();

		return ((len.x + len.y + len.z) / 3.0f);
	}

	Vector AABB::get_center_point() const
	{
		//auto half_len = (dim_lengths() * 0.5f);
		//return { (min.x + half_len.x), (min.y + half_len.y), (min.z + half_len.z) };

		return { ((min.x + max.x) * 0.5f), ((min.y + max.y) * 0.5f), ((min.z + max.z) * 0.5f) };
	}
}