#pragma once

#include "collision_shape_primitive.hpp"

#include <math/types.hpp>

namespace engine
{
	struct CollisionShapeDescription
	{
		CollisionShapePrimitive primitive;

		math::Vector3D size = {};

		inline void set_radius(float radius)
		{
			size = { radius, radius, radius };
		}

		inline float get_radius() const
		{
			//return ((size.x + size.y + size.z) / 3.0f);
			//return size.x; // size.z;

			return size.length();
		}

		inline void set_height(float height)
		{
			size.y = height;
		}

		inline float get_height() const
		{
			return size.y;
		}

		inline void set_xz_size(float xz_size)
		{
			size.x = xz_size;
			size.y = xz_size;
		}

		inline float get_xz_size() const
		{
			return ((size.x + size.z) / 2.0f);
		}
	};
}