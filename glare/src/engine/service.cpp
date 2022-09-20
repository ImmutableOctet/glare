#include "service.hpp"
#include "input_events.hpp"

#include <app/input/types.hpp>

namespace engine
{
	Service::Service(bool register_input_events)
		: active_event_handler(&standard_event_handler)
	{
		if (register_input_events)
		{
			// Only polled device states for now.
			register_event<app::input::MouseState, &Service::on_mouse_input>(*this);
			register_event<app::input::KeyboardState, &Service::on_keyboard_input>(*this);

			// TODO: Implement exact device events, rather than exclusively polled states.
		}
	}

	EventHandler* Service::swap_event_handlers()
	{
		if (active_event_handler == &standard_event_handler)
		{
			return use_forwarding_events();
		}
		else if (active_event_handler == &forwarding_event_handler)
		{
			return use_standard_events();
		}

		return active_event_handler;
	}

	EventHandler* Service::use_standard_events()
	{
		active_event_handler = &standard_event_handler;

		return active_event_handler;
	}

	EventHandler* Service::use_forwarding_events()
	{
		active_event_handler = &forwarding_event_handler;

		return active_event_handler;
	}

	EventHandler& Service::get_active_event_handler()
	{
		assert(active_event_handler);

		return *active_event_handler;
	}

	EventHandler& Service::get_standard_event_handler()
	{
		return standard_event_handler;
	}

	EventHandler& Service::get_forwarding_event_handler()
	{
		return forwarding_event_handler;
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