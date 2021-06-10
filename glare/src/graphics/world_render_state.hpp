#pragma once

#include <optional>

#include "types.hpp"

namespace graphics
{
	struct WorldRenderState
	{
		std::optional<NamedTextureGroupRaw> dynamic_textures = std::nullopt;

		struct
		{
			std::optional<LightPositions> light_positions;
			std::optional<FloatValues>    far_planes;

			bool enabled = false;

			inline operator bool() const { return enabled; }
			//inline operator bool() const { return (light_positions.has_value() || far_planes.has_value()); }
		} point_shadows;

		struct
		{
			std::optional<LightPositions> light_positions;
			std::optional<LightMatrices>  light_matrices;

			bool enabled = false;

			inline operator bool() const { return enabled; }
			//inline operator bool() const { return (light_positions.has_value() || light_matrices.has_value()); }
		} directional_shadows;

		struct
		{
			float min_layers = 8.0;
			float max_layers = 32.0;
		} parallax;

		struct _matrices
		{
			Matrix view;
			Matrix projection;
		};

		std::optional<_matrices> matrices;

		struct
		{
			// Position of camera/view being rendered.
			std::optional<Vector> view_position;
			std::optional<graphics::ColorRGB> ambient_light;
		} meta;

		struct _screen
		{
			float fov_y;
			float aspect_ratio;

			Vector2D depth_range;
		};

		std::optional<_screen> screen;
	};
}