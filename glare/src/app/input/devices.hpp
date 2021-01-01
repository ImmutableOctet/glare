#pragma once

#include "input_device.hpp"
#include "mouse.hpp"
#include "keyboard.hpp"

namespace app::input
{
	struct InputDevices
	{
		Mouse mouse;
		Keyboard keyboard;
	};
}