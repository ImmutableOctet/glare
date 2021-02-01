#include "spin_component.hpp"

#include <engine/world/world.hpp>
#include <engine/transform.hpp>

#include <app/input/types.hpp>
//#include <math/math.hpp>

#include <app/input/keycodes.hpp>

// Debugging related:
#include <iostream>

namespace engine
{
	void SpinBehavior::update(World& world)
	{
		auto& registry = world.get_registry();

		registry.view<SpinBehavior>().each([&](auto entity, auto& spin) // TransformComponent, ...
		{
			auto transform = world.get_transform(entity);

			spin.apply(world, entity, transform);
		});
	}

	void SpinBehavior::apply(World& world, Entity entity, Transform& transform)
	{
		auto delta = world.delta();

		transform.rotate((spin_vector * delta), true);
	}
}