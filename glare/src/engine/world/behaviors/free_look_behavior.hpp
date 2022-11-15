#pragma once

//#include <app/input/mouse_state.hpp>

#include <engine/types.hpp>

namespace app
{
	namespace input
	{
		struct MouseState;
	}
}

namespace engine
{
	struct Transform;

	// Direct mouse-input driven rotation. Useful for debugging and FPS control schemes.
	struct FreeLookBehavior
	{
		using MouseState = app::input::MouseState;

		static void on_mouse(World& world, float delta, const MouseState& mouse_state);

		// Mouse sensitivity. (In radians)
		float sensitivity = glm::radians(0.05f);

		void apply(World& world, Entity entity, Transform& transform, const MouseState& input);
	};
}