#include "rave_component.hpp"

#include "world.hpp"

#include <engine/model_component.hpp>
#include <app/delta_time.hpp>

#include <cmath>

namespace engine
{
	void RaveComponent::update(World& world)
	{
		auto& registry = world.get_registry();

		registry.view<ModelComponent, RaveComponent>().each([&](auto entity, auto& model_comp, auto& rave_comp) // TransformComponent, ...
		{
			rave_comp.apply(world, entity, model_comp);
		});
	}

	void RaveComponent::apply(World& world, Entity entity, ModelComponent& model)
	{
		if (!enabled)
		{
			return;
		}

		const auto& dt = world.get_delta_time();
		
		auto t = static_cast<float>(dt.current_frame_time()) / 600.0f;

		model.color = { std::sinf(t), std::cosf(t), std::tanf(t), 1.0f };
	}
}
