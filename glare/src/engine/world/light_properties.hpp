#pragma once

//#include <math/types.hpp>
#include <graphics/math_types.hpp>

#include <cmath>

namespace engine
{
	struct LightProperties
	{
		inline static float calculate_max_brightness(const graphics::ColorRGB& diffuse)
		{
			return std::fmaxf(std::fmaxf(diffuse.r, diffuse.g), diffuse.b);
		}

		graphics::ColorRGB ambient  = { 0.1f, 0.1f, 0.1f };
		graphics::ColorRGB diffuse  = { 1.0f, 1.0f, 1.0f }; // color;
		graphics::ColorRGB specular = { 0.1f, 0.1f, 0.1f };

		inline float max_brightness() const
		{
			return calculate_max_brightness(diffuse);
		}
	};
}