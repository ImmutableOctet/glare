#include "input_handler.hpp"

#include <utility>

namespace app::input
{
	InputHandler::InputHandler(bool locked)
		: locked(locked)
	{}

	void InputHandler::set_lock_status(bool value)
	{
		locked = value;
	}

	bool InputHandler::get_lock_status() const
	{
		return locked;
	}

	bool InputHandler::process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler)
	{
		const auto mk_event_handler = ((locked) ? opt_event_handler : nullptr);

		if (devices.mouse.process_event(e, mk_event_handler))
		{
			return true;
		}

		if (devices.keyboard.process_event(e, mk_event_handler))
		{
			return true;
		}

		if (devices.gamepads.process_event(e, opt_event_handler))
		{
			return true;
		}

		return false;
	}

	const InputDevices& InputHandler::poll(entt::dispatcher* opt_event_handler)
	{
		if (const auto mk_event_handler = ((locked) ? opt_event_handler : nullptr))
		{
			devices.mouse.poll(mk_event_handler);
			devices.keyboard.poll(mk_event_handler);
		}
		else
		{
			devices.mouse.clear_state();
			devices.keyboard.clear_state();
		}

		devices.gamepads.poll(opt_event_handler);

		return devices;
	}
}