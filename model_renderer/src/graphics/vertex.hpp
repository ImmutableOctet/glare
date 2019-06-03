#pragma once

#include <math/math.hpp>

namespace graphics
{
	struct vertex
	{
		math::vec3f position;
	};

	struct rgb_vertex : public vertex { math::vec3f color; };
	struct rgba_vertex : public vertex { math::vec4f color; };

	struct textured_vertex { math::vec2f uv; };
}