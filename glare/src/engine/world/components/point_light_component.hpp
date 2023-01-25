#pragma once

#include <engine/world/light_properties.hpp>

#include <math/math.hpp>
//#include <cmath>

namespace engine
{
	struct PointLightComponent
	{
		inline static constexpr float DEFAULT_CONSTANT = 1.0f;
		inline static constexpr float DEFAULT_LINEAR = 0.001f; // 0.1f;
		inline static constexpr float DEFAULT_QUADRATIC = 0.000005f; //(1.0f / 2000.0f);

		float constant = DEFAULT_CONSTANT;
		float linear = DEFAULT_LINEAR;
		float quadratic = DEFAULT_QUADRATIC;

		// Explicit floating-point version of `get_radius`. (Needed to simplify reflection)
		inline float get_radius_f(float max_brightness) const
		{
			return (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * max_brightness))) / (2.0f * quadratic);
		}

		inline float get_radius(float max_brightness) const
		{
			return get_radius_f(max_brightness);
		}

		inline float get_radius(const LightProperties& properties) const
		{
			return get_radius(properties.max_brightness());
		}
	};
}