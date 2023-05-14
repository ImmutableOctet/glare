#pragma once

#include <engine/world/types.hpp>

#include <math/math.hpp>

#include <graphics/viewport.hpp>

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

		// Vertical FOV. (In radians)
		float fov;

		float near_plane;
		float far_plane;

		// Display aspect ratio. (Width / height)
		float aspect_ratio;

		Projection projection_mode;

		// Flags:
		bool free_rotation        : 1;
		bool dynamic_aspect_ratio : 1;

		inline constexpr CameraComponent
		(
			float v_fov_deg=75.0f,

			float near_plane=0.1f,
			float far_plane=4000.0f,

			float aspect_ratio=(16.0f / 9.0f), // calculate_aspect_ratio(16, 9), // 16:9
			
			Projection projection_mode=Projection::Default, // Projection::Perspective
			
			bool free_rotation=true,
			bool dynamic_aspect_ratio=true
		) :
			fov(glm::radians(v_fov_deg)),
			near_plane(near_plane),
			far_plane(far_plane),
			aspect_ratio(aspect_ratio),
			projection_mode(projection_mode),
			free_rotation(free_rotation),
			dynamic_aspect_ratio(dynamic_aspect_ratio)
		{}

		inline constexpr auto update_aspect_ratio(int width, int height)
		{
			auto ratio = calculate_aspect_ratio(width, height);

			this->aspect_ratio = ratio;

			return ratio;
		}

		inline math::Matrix get_orthographic_projection(const graphics::Viewport& viewport) const
		{
			const auto width = static_cast<float>(viewport.get_width());
			const auto height = static_cast<float>(viewport.get_height());

			const auto hw = (width / 2.0f);
			const auto hh = (height / 2.0f);

			//return glm::ortho(-hw, hw, hh, -hh, near_plane, far_plane);
			return glm::ortho(-hw, hw, -hh, hh, near_plane, far_plane);
		}

		inline math::Matrix get_perspective_projection() const
		{
			return glm::perspective(fov, aspect_ratio, near_plane, far_plane);
		}
		
		inline math::Matrix get_projection(const graphics::Viewport& viewport) const
		{
			switch (projection_mode)
			{
				case CameraProjection::Orthographic:
				{
					return get_orthographic_projection(viewport);
				}
			}

			return get_perspective_projection();
		}

		inline constexpr float get_vertical_fov() const { return glm::degrees(fov); }
		inline constexpr void set_vertical_fov(float v_fov_deg) { fov = glm::radians(v_fov_deg); }

		inline constexpr bool get_free_rotation() const { return free_rotation; }
		inline constexpr void set_free_rotation(bool value) { free_rotation = value; }

		inline constexpr bool get_dynamic_aspect_ratio() const { return dynamic_aspect_ratio; }
		inline constexpr void set_dynamic_aspect_ratio(bool value) { dynamic_aspect_ratio = value; }
	};
}