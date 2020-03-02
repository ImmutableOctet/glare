#pragma once

//#include <math/math.hpp>

#include "entity.hpp"

namespace engine
{
	struct CameraParameters
	{
		static constexpr float NEAR = 0.1f;
		static constexpr float FAR  = 1000.0f;
		static constexpr float ASPECT = (16.0f / 9.0f); // 16:9

		static constexpr float DEFAULT_FOV = 75.0f;

		CameraParameters(float v_fov_deg=DEFAULT_FOV, float aspect_ratio=ASPECT, float near=NEAR, float far=FAR);

		float fov; // Vertical FOV (Radians)
		float aspect_ratio;
		float near;
		float far;
	};

	Entity create_camera(World& world, float v_fov_deg=CameraParameters::DEFAULT_FOV, Entity parent=null);
}