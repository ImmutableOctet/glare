#pragma once

//#include <math/math.hpp>

#include "entity.hpp"

namespace engine
{
	struct CameraParameters // Component type.
	{
		inline static constexpr float calculate_aspect_ratio(int width, int height)
		{
			return (static_cast<float>(width) / static_cast<float>(height));
		}

		static constexpr float NEAR = 0.1f;
		static constexpr float FAR  = 4000.0f;
		static constexpr float ASPECT = (16.0f / 9.0f); // calculate_aspect_ratio(16, 9); // 16:9

		static constexpr float DEFAULT_FOV = 75.0f;

		float fov; // Vertical FOV (Radians)
		float aspect_ratio;
		float near;
		float far;

		CameraParameters(float v_fov_deg=DEFAULT_FOV, float near=NEAR, float far=FAR, float aspect_ratio=ASPECT);

		inline auto update_aspect_ratio(int width, int height)
		{
			auto ratio = calculate_aspect_ratio(width, height);

			this->aspect_ratio = ratio;

			return ratio;
		}
	};

	Entity create_camera(World& world, CameraParameters params={}, Entity parent=null);
}