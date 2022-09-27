#include "input_handler.hpp"

namespace app::input
{
	const InputDevices& InputHandler::poll(engine::EventHandler& event_handler)
	{
		devices.mouse.poll(event_handler);
		devices.keyboard.poll(event_handler);
		devices.gamepads.poll(event_handler);

		return devices;
	}
}