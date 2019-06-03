#pragma once

#include <math/math.hpp>

namespace graphics
{
	struct Vertex
	{
		math::vec3f position;
	};

	struct RGBVertex : public Vertex { math::vec3f color; };
	struct RBAVertex : public Vertex { math::vec4f color; };

	struct TextureVertex : public Vertex { math::vec2f uv; };
}