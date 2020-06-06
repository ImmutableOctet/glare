#include "light.hpp"
#include <engine/world/world.hpp>

namespace engine
{
	Entity create_light(World& world, Entity parent)
	{
		auto light = create_pivot(world, parent);
		world.get_registry().assign<LightComponent>(light);

		return light;
	}
}