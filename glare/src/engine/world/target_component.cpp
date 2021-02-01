#include "target_component.hpp"
#include "world.hpp"

namespace engine
{
	void TargetComponent::update(World& world)
	{
		auto& registry = world.get_registry();

		registry.view<TargetComponent>().each([&](auto entity, auto& target_comp) // TransformComponent, ...
		{
			auto transform = world.get_transform(entity);

			target_comp.apply(world, entity, transform);
		});
	}

	void TargetComponent::apply(World& world, Entity entity, Transform& transform)
	{
		auto delta = world.delta();
		auto& registry = world.get_registry();

		auto target_transform = world.get_transform(target);

		transform.set_matrix(transform.get_matrix() * glm::lookAt(transform.get_position(), target_transform.get_position(), world.get_up_vector()));
	}
}