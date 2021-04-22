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
			std::optional<LightPositions> light_positions  = std::nullopt;
			std::optional<FloatValues>    far_planes       = std::nullopt;
		} point_shadows;

		struct
		{
			std::optional<LightPositions> light_positions  = std::nullopt;
			std::optional<LightMatrices>  light_matrices   = std::nullopt;
		} directional_shadows;

		struct
		{
			// Position of camera/view being rendered.
			const Vector* view_position = nullptr;
		} meta;
	};
}