#pragma once

#include <engine/world/entity.hpp>
#include <graphics/types.hpp>

namespace engine
{
	struct LightComponent
	{
		LightType type = LightType::Point;

		graphics::ColorRGB color = { 1.0f, 1.0f, 1.0f };
	};

	Entity create_light(World& world, const math::Vector& position, const graphics::ColorRGB& color, LightType type=LightType::Point, Entity parent=null);
}