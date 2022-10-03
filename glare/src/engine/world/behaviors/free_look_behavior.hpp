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

	struct FreeLookBehavior
	{
		using MouseState = app::input::MouseState;

		static void on_mouse(World& world, float delta, const MouseState& mouse_state);

		//int x = 0;
		//int y = 0;

		// Mouse sensitivity. (In radians)
		float sensitivity = glm::radians(0.05f);

		void apply(World& world, Entity entity, Transform& transform, const MouseState& input);
	};
}