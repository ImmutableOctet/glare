#pragma once

#include <math/types.hpp>

namespace graphics
{
	/*
	struct Point
	{
		union
		{
			struct { int x, y; };
			//struct { int width, height; };
		};
	};
	*/

	using Point = math::vec2i; // math::Vector2D;
}