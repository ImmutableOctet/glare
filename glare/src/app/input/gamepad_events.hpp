#pragma once

#include "types.hpp"
#include "gamepad_analog.hpp"
#include "gamepad_state.hpp"

namespace app::input
{
	// Gamepad event types:

	// Triggered automatically by a `GamepadManager` when a gamepad connects.
	struct OnGamepadConnected
	{
		// The device that connected.
		GamepadDeviceIndex device_index;
	};

	// Triggered automatically by a `GamepadManager` when a gamepad disconnects.
	struct OnGamepadDisconnected
	{
		// The device that disconnected.
		GamepadDeviceIndex device_index;
	};

	// Base event type; not triggered directly.
	// (Not currently aliased, either)
	struct GamepadStateEvent : public InputStateEvent<GamepadDeviceIndex, GamepadState> {};

	// Base event type for button-related actions.
	// (Not currently aliased or triggered directly)
	struct GamepadButtonEvent : public GamepadStateEvent
	{
		// TODO: Look into changing this to use the `GamepadButton` enum instead.
		// The button that has been pressed, held, or released.
		GamepadButtonID button;
	};

	//  Triggered when a button is pressed or held.
	struct OnGamepadButtonDown : public GamepadButtonEvent {};

	//  Triggered when a button is released.
	struct OnGamepadButtonUp : public GamepadButtonEvent {};

	// Triggered for analog inputs (Left, Right and Triggers), as well as D-Pad usage.
	struct OnGamepadAnalogInput : public GamepadStateEvent
	{
		// The analog source this input is coming from.
		GamepadAnalog analog;

		/*
			For all input types except for `GamepadAnalog::Triggers`,
			this represents the normalized XY direction of the input.
			
			For `GamepadAnalog::Triggers` in particular, `x` represents
			the left trigger, whilst `y` represents the right.
		*/
		math::Vector2D value;

		// The angle of the direction-vector `value`.
		// For `GamepadAnalog::Triggers`, this always returns zero.
		float angle() const;
	};
}