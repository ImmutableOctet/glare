#pragma once

#include "mouse.hpp"
#include "keyboard.hpp"

namespace app
{
	namespace input
	{
		struct InputState
		{
			Mouse mouse;
			Keyboard keyboard;
		};
	}
}