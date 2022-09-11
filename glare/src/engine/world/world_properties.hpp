#pragma once

#include <graphics/types.hpp>
#include <math/math.hpp>
#include <util/json.hpp>

namespace engine
{
	// Global values applied to a `World`.
	struct WorldProperties
	{
		graphics::ColorRGB ambient_light = { 0.8f, 0.8f, 0.8f };
		math::Vector gravity = { 0.0f, -1.0f, 0.0f };

		WorldProperties() = default;
		WorldProperties(const WorldProperties&) = default;
		WorldProperties(WorldProperties&&) noexcept = default;

		inline WorldProperties(const util::json& data) : WorldProperties()
		{
			ambient_light = util::get_color_rgb(data, "ambient_light", ambient_light);
			gravity = util::get_vector(data, "gravity", gravity);
		}

		WorldProperties& operator=(const WorldProperties&) = default;
		WorldProperties& operator=(WorldProperties&&) noexcept = default;
	} ;
}