#pragma once

#include "types.hpp"

#include <app/input/devices.hpp>

namespace app
{
	namespace input
	{
		class InputHandler
		{
			protected:
				InputDevices devices;
			public:
				inline Mouse& get_mouse()       { return devices.mouse;    }
				inline Keyboard& get_keyboard() { return devices.keyboard; }

				const InputDevices& poll(engine::EventHandler& event_handler);
		};
	}
}