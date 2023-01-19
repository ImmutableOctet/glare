#pragma once

#include <engine/world/types.hpp>

//#include <math/math.hpp>

// Forward declaration not used due to templates.
#include <util/json.hpp>

namespace engine
{
	struct CameraComponent
	{
		using Projection = CameraProjection;

		inline static constexpr float calculate_aspect_ratio(int width, int height)
		{
			return (static_cast<float>(width) / static_cast<float>(height));
		}

		inline static constexpr float calculate_aspect_ratio(const math::vec2i& size)
		{
			return calculate_aspect_ratio(size.x, size.y);
		}

		static Projection resolve_projection_mode(const std::string& mode);

		static constexpr float NEAR_PLANE = 0.1f;
		static constexpr float FAR_PLANE  = 4000.0f;
		static constexpr float ASPECT = (16.0f / 9.0f); // calculate_aspect_ratio(16, 9); // 16:9

		static constexpr float DEFAULT_FOV = 75.0f;

		static constexpr bool DEFAULT_FREE_ROTATION = true; // false;
		static constexpr bool DEFAULT_DYNAMIC_ASPECT_RATIO = true; // false;

		// Vertical FOV. (In radians)
		float fov;

		float aspect_ratio;

		float near_plane;
		float far_plane;

		Projection projection_mode;

		// Flags:
		bool free_rotation        : 1;
		bool dynamic_aspect_ratio : 1;

		CameraComponent(const util::json& camera_cfg);

		CameraComponent
		(
			float v_fov_deg=DEFAULT_FOV,
			float near_plane=NEAR_PLANE,
			float far_plane=FAR_PLANE,
			float aspect_ratio=ASPECT,
			
			Projection projection_mode=Projection::Default,
			
			bool free_rotation=DEFAULT_FREE_ROTATION,
			bool dynamic_aspect_ratio=DEFAULT_DYNAMIC_ASPECT_RATIO
		);

		inline auto update_aspect_ratio(int width, int height)
		{
			auto ratio = calculate_aspect_ratio(width, height);

			this->aspect_ratio = ratio;

			return ratio;
		}

		// Getters/setters:
		inline bool get_free_rotation() const { return free_rotation; }
		inline void set_free_rotation(bool value) { free_rotation = value; }

		inline bool get_dynamic_aspect_ratio() const { return dynamic_aspect_ratio; }
		inline void set_dynamic_aspect_ratio(bool value) { dynamic_aspect_ratio = value; }
	};
}