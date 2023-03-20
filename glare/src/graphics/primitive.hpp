#pragma once

#include <cstdint>

namespace graphics
{
	enum class Primitive : std::int8_t
	{
		Point,
		Line,
		LineLoop,
		LineStrip,
		Triangle,
		TriangleStrip,
		TriangleFan,
		Quad,
		QuadStrip,
		Polygon,

		Unknown = -1,
	};
}