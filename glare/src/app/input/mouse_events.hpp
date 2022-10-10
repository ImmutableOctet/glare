#pragma once

#include "types.hpp"

#include "mouse_state.hpp"
#include "mouse_buttons.hpp"
#include "mouse_motion.hpp"

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

	// Base event type for motion/analog actions.
	// (Not currently aliased or triggered directly)
	struct MouseAnalogEvent : public MouseStateEvent
	{
		MouseMotion analog;
	};

	// Triggered when the mouse is moved.
	struct OnMouseMove : public MouseAnalogEvent
	{
		int x, y;
	};

	struct OnMouseScroll : public MouseAnalogEvent
	{
		int wheel_x, wheel_y;
	};
}