#pragma once

#include <math/math.hpp>

namespace engine
{
	class Config
	{
		public:
			using Resolution = math::vec2i;

		protected:
			Resolution shadow_resolution = { 8192, 8192 }; // 2048

		public:
			Resolution get_shadow_resolution() const { return shadow_resolution; }
	};
}