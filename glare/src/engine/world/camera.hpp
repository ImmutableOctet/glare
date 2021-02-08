#pragma once

//#include <math/math.hpp>

#include "entity.hpp"

namespace engine
{
	enum class CameraProjection : std::uint8_t
	{
		Perspective,
		Orthographic,

		Default = Perspective
	};

	struct CameraParameters // Component type.
	{
		using Projection = CameraProjection;

		inline static constexpr float calculate_aspect_ratio(int width, int height)
		{
			return (static_cast<float>(width) / static_cast<float>(height));
		}

		static Projection resolve_projection_mode(const std::string& mode);

		static constexpr float NEAR_PLANE = 0.1f;
		static constexpr float FAR_PLANE  = 4000.0f;
		static constexpr float ASPECT = (16.0f / 9.0f); // calculate_aspect_ratio(16, 9); // 16:9

		static constexpr float DEFAULT_FOV = 75.0f;

		static constexpr bool DEFAULT_FREE_ROTATION = true; // false;

		float fov; // Vertical FOV (Radians)
		float aspect_ratio;
		float near_plane;
		float far_plane;

		Projection projection_mode;

		bool free_rotation;

		CameraParameters(float v_fov_deg=DEFAULT_FOV, float near_plane=NEAR_PLANE, float far_plane=FAR_PLANE, float aspect_ratio=ASPECT, Projection projection_mode=Projection::Default, bool free_rotation=DEFAULT_FREE_ROTATION);

		inline auto update_aspect_ratio(int width, int height)
		{
			auto ratio = calculate_aspect_ratio(width, height);

			this->aspect_ratio = ratio;

			return ratio;
		}
	};

	Entity create_camera(World& world, CameraParameters params={}, Entity parent=null, bool make_active=false);
}