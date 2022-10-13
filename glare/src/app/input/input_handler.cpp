#include "input_handler.hpp"

#include <utility>

namespace app::input
{
	InputHandler::InputHandler(bool events_enabled)
		: events_enabled(events_enabled)
	{}

	void InputHandler::set_event_status(bool value)
	{
		events_enabled = value;
	}

	bool InputHandler::get_event_status() const
	{
		return events_enabled;
	}

	bool InputHandler::process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler)
	{
		return devices.process_event(e, (events_enabled) ? opt_event_handler : nullptr);
	}

	const InputDevices& InputHandler::poll(engine::EventHandler* opt_event_handler)
	{
		return poll_impl((events_enabled) ? opt_event_handler : nullptr);
	}

	const InputDevices& InputHandler::poll_impl(engine::EventHandler* opt_event_handler)
	{
		devices.mouse.poll(opt_event_handler);
		devices.keyboard.poll(opt_event_handler);
		devices.gamepads.poll(opt_event_handler);

		return devices;
	}
}