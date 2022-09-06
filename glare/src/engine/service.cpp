#include "service.hpp"
#include "input_events.hpp"

#include <app/input/types.hpp>

namespace engine
{
	Service::Service(bool register_input_events)
	{
		if (register_input_events)
		{
			// Only polled device states for now.
			register_event<app::input::MouseState, &Service::on_mouse_input>();
			register_event<app::input::KeyboardState, &Service::on_keyboard_input>();

			// TODO: Implement exact device events, rather than exclusively polled states.
		}
	}

	// Input re-submission callbacks (see class declaration(s) for details):
	void Service::on_mouse_input(const app::input::MouseState& mouse)
	{
		event<OnMouseState>(this, &mouse);
	}

	void Service::on_keyboard_input(const app::input::KeyboardState& keyboard)
	{
		event<OnKeyboardState>(this, &keyboard);
	}
}