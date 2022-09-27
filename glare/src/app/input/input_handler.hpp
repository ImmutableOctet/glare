#pragma once

#include "types.hpp"

#include <app/input/devices.hpp>
#include <entt/signal/dispatcher.hpp>

namespace app::input
{
	class InputHandler
	{
		protected:
			InputDevices devices;
		public:
			inline Mouse& get_mouse()             { return devices.mouse;    }
			inline Keyboard& get_keyboard()       { return devices.keyboard; }
			inline GamepadManager& get_gamepads() { return devices.gamepads; }

			inline bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr)
			{
				return devices.process_event(e, opt_event_handler);
			}

			const InputDevices& poll(engine::EventHandler& event_handler);
	};
}