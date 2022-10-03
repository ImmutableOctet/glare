#pragma once

#include "input_device.hpp"
#include "mouse.hpp"
#include "keyboard.hpp"
#include "gamepad.hpp"
#include "gamepad_manager.hpp"

#include <entt/signal/dispatcher.hpp>

namespace app::input
{
	struct InputDevices
	{
		Mouse mouse;
		Keyboard keyboard;
		GamepadManager gamepads;

		inline bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr)
		{
			if (mouse.process_event(e))
			{
				return true;
			}

			if (keyboard.process_event(e))
			{
				return true;
			}

			if (gamepads.process_event(e, opt_event_handler))
			{
				return true;
			}

			return false;
		}
	};
}