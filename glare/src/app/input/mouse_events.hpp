#pragma once

#include "types.hpp"

#include "mouse_state.hpp"
#include "mouse_buttons.hpp"

namespace app::input
{
	// Mouse event types:

	// Base event type; not triggered directly.
	// (Not currently aliased, either)
	struct MouseStateEvent : public InputStateEvent<MouseDeviceIndex, MouseState> {};

	// Base event type for button-related actions.
	// (Not currently aliased or triggered directly)
	struct MouseButtonEvent : public MouseStateEvent
	{
		MouseButton button;
	};

	// Triggered when a mouse button is held down.
	struct OnMouseButtonDown : public MouseButtonEvent {};

	// Triggered when a mouse button is released.
	struct OnMouseButtonUp : public MouseButtonEvent {};

	// Triggered when the mouse is moved.
	struct OnMouseMove : public MouseStateEvent
	{
		int x, y;
	};

	struct OnMouseScroll : public MouseStateEvent
	{
		int wheel_x, wheel_y;
	};
}