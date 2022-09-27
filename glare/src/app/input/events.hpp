#pragma once

#include "types.hpp"
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
	struct GamepadStateEvent
	{
		// The device that triggered this input event.
		GamepadDeviceIndex device_index;

		// A snapshot of the gamepad's state at the time of this event.
		GamepadState state;
	};

	//  Triggered when a button is pressed or held.
	struct OnGamepadButtonDown : public GamepadStateEvent
	{
		// The button that has been held/pressed.
		GamepadButtonID button;
	};

	//  Triggered when a button is released.
	struct OnGamepadButtonUp : public GamepadStateEvent
	{
		// The button that has been released.
		GamepadButtonID button;
	};

	// Triggered for analog inputs (Left, Right and Triggers), as well as D-Pad usage.
	struct OnGamepadAnalogInput : public GamepadStateEvent
	{
		// The analog source this input is coming from.
		GamepadAnalogInput analog;

		/*
			For all input types except for `GamepadAnalogInput::Triggers`,
			this represents the normalized XY direction of the input.
			
			For `GamepadAnalogInput::Triggers` in particular, `x` represents
			the left trigger, whilst `y` represents the right.
		*/
		math::Vector2D value;

		// The angle of the direction-vector `value`.
		// For `GamepadAnalogInput::Triggers`, this always returns zero.
		float angle() const;
	};
}