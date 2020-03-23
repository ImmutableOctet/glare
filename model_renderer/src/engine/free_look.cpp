#include "free_look.hpp"

#include "world/world.hpp"
#include "transform.hpp"

#include <app/input/types.hpp>
//#include <math/math.hpp>
//#include <app/input/mouse/mouse.hpp>

namespace engine
{
	void FreeLook::update(World& world, const MouseState& mouse_state)
	{
		auto& registry = world.get_registry();

		registry.view<FreeLook>().each([&](auto entity, auto& free_look)
		{
			auto transform = world.get_transform(entity);

			free_look.apply(world, entity, transform, mouse_state);
		});
	}

	void FreeLook::apply(World& world, Entity entity, Transform& transform, const FreeLook::MouseState& input)
	{
		auto sens = this->sensitivity;

		transform.rotate({ 0.0f, (sens * input.x), 0.0f }, true); // false
		transform.rotateX((-sens * input.y), false); // true
	}
}