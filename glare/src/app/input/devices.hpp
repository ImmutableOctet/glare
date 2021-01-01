#pragma once

#include "input_device.hpp"
#include "mouse/mouse.hpp"
#include "keyboard/keyboard.hpp"

namespace app
{
	namespace input
	{
		struct InputDevices
		{
			Mouse mouse;
			Keyboard keyboard;
		};
	}
}