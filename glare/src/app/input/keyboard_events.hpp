#pragma once

#include "types.hpp"

#include "keyboard_state.hpp"
#include "keyboard_buttons.hpp"
#include "keyboard_motion.hpp"

#include <math/types.hpp>

namespace app::input
{
	// Base event type; not triggered directly.
	// (Not currently aliased, either)
	struct KeyboardStateEvent : public InputStateEvent<KeyboardDeviceIndex, KeyboardState> {};

	// Base event type for button-related actions.
	// (Not currently aliased or triggered directly)
	struct KeyboardButtonEvent : public KeyboardStateEvent
	{
		KeyboardButton button;
	};

	// Triggered when a keyboard button is held down.
	struct OnKeyboardButtonDown : public KeyboardButtonEvent {};

	// Triggered when a keyboard button is released.
	struct OnKeyboardButtonUp : public KeyboardButtonEvent {};

	// Hat-based input from a `Keyboard` device.
	struct OnKeyboardAnalogInput : public KeyboardStateEvent
	{
		// The 'analog' source (Hat index) this input is coming from.
		KeyboardMotion analog;

		// A normalized direction vector.
		math::Vector2D value;

		// The angle of the direction-vector, `value`.
		// For `GamepadAnalog::Triggers`, this always returns zero.
		float angle() const;
	};
}