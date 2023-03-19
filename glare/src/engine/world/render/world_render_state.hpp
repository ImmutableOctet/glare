#pragma once

#include <graphics/types.hpp>
#include <graphics/texture_array.hpp>
#include <graphics/light_types.hpp>
#include <graphics/array_types.hpp>

#include <optional>

namespace engine
{
	// View of state from previous rendering pipeline stages.
	struct WorldRenderState
	{
		// Temporary reference to a dictionary of dynamically updated texture maps. (e.g. shadow maps)
		//std::optional<graphics::NamedTextureGroupRaw> dynamic_textures = std::nullopt;
		graphics::NamedTextureArrayRaw* dynamic_textures = nullptr;

		struct
		{
			std::optional<graphics::LightPositions> light_positions;
			std::optional<graphics::FloatValues>    far_planes;

			bool enabled = false;

			inline operator bool() const { return enabled; }
			//inline operator bool() const { return (light_positions.has_value() || far_planes.has_value()); }
		} point_shadows;

		struct
		{
			std::optional<graphics::LightPositions> light_positions;
			std::optional<graphics::LightMatrices>  light_matrices;

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
			graphics::Matrix view;
			graphics::Matrix projection;
		};

		std::optional<_matrices> matrices;

		struct
		{
			// Position of camera/view being rendered.
			std::optional<math::Vector> view_position;
			std::optional<graphics::ColorRGB> ambient_light;
		} meta;

		struct _screen
		{
			float fov_y;
			float aspect_ratio;

			math::Vector2D depth_range;
		};

		std::optional<_screen> screen;
	};
}