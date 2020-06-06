#pragma once

#include <engine/world/entity.hpp>

namespace engine
{
	struct LightComponent
	{
		LightType type = LightType::Point;
	};

	Entity create_light(World& world, Entity parent=null);
}