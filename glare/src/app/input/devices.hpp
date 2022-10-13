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
	};
}