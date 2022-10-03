#include "input_handler.hpp"

#include <utility>

namespace app::input
{
	InputHandler::InputHandler(EngineButtonMap&& button_name_to_value) :
		buttons(std::move(button_name_to_value))
	{}

	const InputDevices& InputHandler::poll(engine::EventHandler& event_handler)
	{
		devices.mouse.poll(event_handler);
		devices.keyboard.poll(event_handler);
		devices.gamepads.poll(event_handler);

		return devices;
	}
}