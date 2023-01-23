#pragma once

#include "components/camera_component.hpp"

#include <math/types.hpp>

#include <optional>

namespace engine
{
	bool attach_shadows
	(
		World& world,
		
		Entity light,
		LightType light_type,
		
		std::optional<math::vec2i> resolution_2d=std::nullopt,
		std::optional<math::vec2i> cube_map_resolution=std::nullopt,
		std::optional<CameraComponent> perspective_cfg=std::nullopt,

		bool update_aspect_ratio=true,
		bool conform_to_light_type=false
	);

	// Update shadow-map state (LightShadows, etc.) based on the type of light specified.
	void update_shadows(World& world, Entity light);
}