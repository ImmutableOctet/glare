#include "free_look_behavior.hpp"

#include <engine/world/world.hpp>
#include <engine/transform.hpp>

//#include <app/input/mouse/mouse.hpp>
#include <app/input/mouse_state.hpp>

//#include <math/math.hpp>

namespace engine
{
	void FreeLookBehavior::on_mouse(World& world, float delta, const MouseState& mouse_state)
	{
		auto& registry = world.get_registry();

		registry.view<FreeLookBehavior>().each([&](auto entity, auto& free_look)
		{
			auto transform = world.get_transform(entity);

			free_look.apply(world, entity, transform, mouse_state);
		});
	}

	void FreeLookBehavior::apply(World& world, Entity entity, Transform& transform, const FreeLookBehavior::MouseState& input)
	{
		auto sens = this->sensitivity;

		transform.rotate({ 0.0f, (sens * input.x), 0.0f }, true); // false <-- Need to verify if this behaves correctly when set to 'false'.
		transform.rotateX((-sens * input.y), false); // true
	}
}