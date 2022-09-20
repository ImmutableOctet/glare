#include "billboard_behavior.hpp"

#include <engine/world/world.hpp>

namespace engine
{
	void BillboardBehavior::on_update(World& world, float delta)
	{
		auto& registry = world.get_registry();

		registry.view<BillboardBehavior>().each([&](auto entity, auto& bb_comp) // TransformComponent, ...
		{
			auto transform = world.get_transform(entity);

			bb_comp.apply(world, entity, transform, delta);
		});
	}

	void BillboardBehavior::apply(World& world, Entity entity, Transform& transform, float delta, bool update_target)
	{
		if (!enabled)
		{
			return;
		}

		if (update_target)
		{
			auto camera = world.get_camera();

			target = camera;

			if (camera == null)
			{
				return;
			}
		}

		TargetBehavior::apply(world, entity, transform, delta);
	}
}