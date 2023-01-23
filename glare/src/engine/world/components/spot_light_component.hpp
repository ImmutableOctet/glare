#pragma once

#include <math/math.hpp>

namespace engine
{
	struct SpotLightComponent
	{
		inline static constexpr float DEFAULT_CONSTANT = 1.0f;
		inline static constexpr float DEFAULT_LINEAR = 0.09f;
		inline static constexpr float DEFAULT_QUADRATIC = 0.032f;

		inline static float DEFAULT_CUTOFF() { return glm::cos(glm::radians(12.5f)); }; // constexpr
		inline static float DEFAULT_OUTER_CUTOFF() { return glm::cos(glm::radians(17.5f)); }; // constexpr

		float constant = DEFAULT_CONSTANT;
		float linear = DEFAULT_LINEAR;
		float quadratic = DEFAULT_QUADRATIC;
		
		float cutoff = DEFAULT_CUTOFF();
		float outer_cutoff = DEFAULT_OUTER_CUTOFF();
	};
}