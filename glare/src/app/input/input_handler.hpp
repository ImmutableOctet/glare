#pragma once

#include "types.hpp"

#include <app/input/devices.hpp>
#include <entt/signal/fwd.hpp>

namespace app::input
{
	class InputHandler
	{
		protected:
			InputDevices devices;

			// Mappings of game/engine buttons to their formal names used in configuration files.
			EngineButtonMap buttons;

			// Mappings of game/engine analogs to their formal names used in configuration files.
			EngineAnalogMap analogs;

			// Mapping of device names to their corresponding player identifiers.
			PlayerDeviceMap player_device_names;

			bool locked : 1 = true;
		public:
			InputHandler(bool locked=true);

			//InputHandler(const InputHandler&) = default;
			//InputHandler(InputHandler&&) noexcept = default;

			inline const InputDevices& get_devices() const { return devices; }

			inline Mouse& get_mouse()             { return devices.mouse;    }
			inline Keyboard& get_keyboard()       { return devices.keyboard; }
			inline GamepadManager& get_gamepads() { return devices.gamepads; }

			// Retireves the `buttons` container for modification. (Addition of entries, etc.)
			inline EngineButtonMap& get_buttons() { return buttons; }

			// Retrieves the `buttons` container for inspection, rather than modification.
			inline const EngineButtonMap& get_buttons() const { return buttons; }

			// Retireves the `analogs` container for modification. (Addition of entries, etc.)
			inline EngineAnalogMap& get_analogs() { return analogs; }

			// Retrieves the `analogs` container for inspection, rather than modification.
			inline const EngineAnalogMap& get_analogs() const { return analogs; }

			// Retrieves the `player_device_names` container for inspection, rather than modification.
			inline const PlayerDeviceMap& get_player_device_map() const { return player_device_names; }

			// Retrieves the `player_device_names` container for modification.
			// (Useful for population during a configuration loading routine, etc.)
			inline PlayerDeviceMap& get_player_device_map() { return player_device_names; }

			// Sets the lock status for mouse and keyboard devices.
			void set_lock_status(bool value);

			// Indicates the current lock status.
			bool get_lock_status() const;

			// Handles an SDL input event. If the event type of `e` is not handled, this will return false.
			bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr);

			// Polls input on all device types.
			const InputDevices& poll(entt::dispatcher* opt_event_handler);
	};
}