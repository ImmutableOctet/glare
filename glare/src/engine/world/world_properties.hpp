#pragma once

#include <graphics/types.hpp>
#include <math/types.hpp>
#include <util/json.hpp>

namespace engine
{
	// Global values applied to a `World`.
	struct WorldProperties
	{
		graphics::ColorRGB ambient_light = { 0.8f, 0.8f, 0.8f };
		math::Vector gravity = { 0.0f, -1.0f, 0.0f };

		// TODO: Remove.
		inline static WorldProperties from_json(const util::json& data)
		{
			WorldProperties properties;

			properties.ambient_light = util::get_color_rgb(data, "ambient_light", properties.ambient_light);
			properties.gravity = util::get_vector(data, "gravity", properties.gravity);

			return properties;
		}
	} ;
}