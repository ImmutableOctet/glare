#pragma once

#include <math/math.hpp>

namespace engine
{
	class Config
	{
		public:
			using Resolution = math::vec2i;

		protected:
			Resolution shadow_resolution = { 8192, 8192 };
			Resolution shadow_cubemap_resolution = { 2048, 2048 };

		public:
			Resolution get_shadow_resolution() const { return shadow_resolution; }
			Resolution get_shadow_cubemap_resolution() const { return shadow_cubemap_resolution; }
	};
}