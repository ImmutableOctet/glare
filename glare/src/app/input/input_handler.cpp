#include "input_handler.hpp"

#include <utility>

namespace app::input
{
	InputHandler::InputHandler(EngineButtonMap&& button_name_to_value) :
		buttons(std::move(button_name_to_value))
	{}

	const InputDevices& InputHandler::poll(engine::EventHandler* opt_event_handler)
	{
		devices.mouse.poll(opt_event_handler);
		devices.keyboard.poll(opt_event_handler);
		devices.gamepads.poll(opt_event_handler);

		return devices;
	}
}