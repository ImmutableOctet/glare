#include "light.hpp"
#include <engine/world/world.hpp>

namespace engine
{
	Entity create_light(World& world, const math::Vector& position, const graphics::ColorRGB& color, LightType type, Entity parent)
	{
		auto light = create_pivot(world, position, parent);

		world.get_registry().emplace<LightComponent>(light, type, color);

		return light;
	}
}