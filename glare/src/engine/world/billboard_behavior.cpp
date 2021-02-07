#include "billboard_behavior.hpp"
#include "world.hpp"

namespace engine
{
	void BillboardBehavior::update(World& world)
	{
		auto& registry = world.get_registry();

		registry.view<BillboardBehavior>().each([&](auto entity, auto& bb_comp) // TransformComponent, ...
		{
			auto transform = world.get_transform(entity);

			bb_comp.apply(world, entity, transform);
		});
	}

	void BillboardBehavior::apply(World& world, Entity entity, Transform& transform, bool update_target)
	{
		if (!enabled)
		{
			return;
		}

		if (update_target)
		{
			auto camera = world.get_camera();

			if (camera == null)
			{
				return;
			}

			target = camera;
		}

		TargetComponent::apply(world, entity, transform);
	}
}