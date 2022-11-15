#include "rave_behavior.hpp"

#include <engine/world/world.hpp>
#include <engine/components/model_component.hpp>

#include <app/delta_time.hpp>

#include <cmath>

namespace engine
{
	void RaveBehavior::on_update(World& world, float delta)
	{
		auto& registry = world.get_registry();

		registry.view<ModelComponent, RaveBehavior>().each([&](auto entity, auto& model_comp, auto& rave_comp) // TransformComponent, ...
		{
			rave_comp.apply(world, entity, model_comp);
		});
	}

	void RaveBehavior::apply(World& world, Entity entity, ModelComponent& model)
	{
		// If the component is disabled, we currently leave the `model.color` field as it was.
		// This means that toggling this component would leave the last color generated, rather than reverting back to 'normal'.
		if (!enabled)
		{
			return;
		}

		// TODO: Look into changing this implementation to use a
		// global variable or component-local variable, etc.
		// (instead of using `app::DeltaTime` directly)
		const auto& dt = world.get_delta_time();
		
		auto t = static_cast<float>(dt.current_frame_time()) / 600.0f;

		model.color = { std::sinf(t), std::cosf(t), std::tanf(t), 1.0f };
	}
}
