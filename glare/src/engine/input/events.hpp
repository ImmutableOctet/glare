#pragma once

#include "types.hpp"
#include "input_event.hpp"
#include "buttons.hpp"
#include "analogs.hpp"
#include "raw_input_events.hpp"

// TODO: Determine if we actually want these included here.
//#include <app/input/events.hpp> // <-- 'Raw' input events. (Directly from devices)

namespace engine
{
	// Triggered once per-update/frame if the high-level input state has changed.
	struct OnInput : public InputEvent
	{
		// The previous input state.
		InputState previous_state;
	};

	// Abstract common base-type for button-related input events.
	// This type is not used directly as a triggerred event, nor is it aliased.
	struct ButtonEvent : public InputEvent
	{
		Button button;
	};

	/*
		This is triggered once, when a button is pressed or held.

		Unlike `OnButtonDown`, this is not a continuously fired event.
		
		Unlike `OnButtonReleased`, this does not require an additional (release) action from the user.
		As a consequence `OnButtonPressed` does not have any implicit delay.
	*/
	struct OnButtonPressed : public ButtonEvent {};

	/*
		Triggered once when a button is no longer being held down (released).
		
		This differs from `OnButtonPressed` since this requires the button to be released,
		which can take an undetermined amount of time. Additionally, a button does not
		need to be held for more than one update/frame to be considered released.
		
		This event commonly follows a series of `OnButtonDown` events.
	*/
	struct OnButtonReleased : public ButtonEvent {};

	/*
		This is triggered repeatedly whenever a button is held down, regardless of how long.
		
		e.g. A `OnButtonPressed` event is triggered once, at the exact time as a button starts being held down,
		but an `OnButtonDown` event fires continuously as long as the button is being held.
	*/
	struct OnButtonDown : public ButtonEvent {};

	// Alternate name for `OnButtonDown`.
	using OnButtonHeld = OnButtonDown;

	// An analog input is abstraction of any directional or 'variable strength' input from a device, regardless of its capabilities.
	// i.e. a keyboard could simulate an analog input using WASD, without a physical analog stick,
	// but something like a gamepad could map the same thing to its physical analog controls.
	struct OnAnalogInput : public InputEvent
	{
		// The type of analog input.
		Analog analog;

		// The analog values received.
		math::Vector2D value;

		// The angle of `value`. (if applicable)
		float angle;
	};
}