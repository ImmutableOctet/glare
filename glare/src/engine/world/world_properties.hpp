#pragma once

#include <graphics/types.hpp>
#include <math/math.hpp>
#include <util/json.hpp>

namespace engine
{
	// Global values for a `World`. This type should be treated as read-only
	// when accessed outside of `World`.
	struct WorldProperties
	{
		graphics::ColorRGB ambient_light = { 0.8f, 0.8f, 0.8f };
		math::Vector gravity = { 0.0f, -1.0f, 0.0f };

		WorldProperties() = default;

		inline WorldProperties(const util::json& data) : WorldProperties()
		{
			ambient_light = util::get_color_rgb(data, "ambient_light", ambient_light);
			gravity = util::get_vector(data, "gravity", gravity);
		}
	} ;
}