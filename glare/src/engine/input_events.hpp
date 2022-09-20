#pragma once

namespace app::input
{
	struct MouseState;
	struct KeyboardState;
}

namespace engine
{
	class Service;

	// TODO: Look into having keyup/keydown events as well.

	// Fired immediately once a new keyboard state has been polled.
	// (Less useful for event-based keyboard input, but good for debugging purposes)
	struct OnKeyboardState
	{
		using State = app::input::KeyboardState;

		Service* service;
		const State* keyboard_state;
	};

	// Fired immediately once a new mouse state has been polled.
	// (Useful for detecting the mouse's position)
	struct OnMouseState
	{
		using State = app::input::MouseState;

		Service* service;
		const State* mouse_state;
	};
}