#include "spin_component.hpp"

#include <engine/world/world.hpp>
#include <engine/transform.hpp>

namespace engine
{
	void SpinBehavior::on_update(World& world, float delta)
	{
		auto& registry = world.get_registry();

		registry.view<SpinBehavior>().each([&](auto entity, auto& spin) // TransformComponent, ...
		{
			auto transform = world.get_transform(entity);

			spin.apply(world, entity, transform);
		});
	}

	void SpinBehavior::apply(World& world, Entity entity, Transform& transform, float delta)
	{
		transform.rotate((spin_vector * delta), true);
	}
}