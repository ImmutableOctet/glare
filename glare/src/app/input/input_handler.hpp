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

			// Mappings of game/engine buttons to their formal names used in configuration files.
			EngineButtonMap buttons;

			// Mappings of game/engine analogs to their formal names used in configuration files.
			EngineAnalogMap analogs;

			// Mapping of device names to their corresponding player identifiers.
			PlayerDeviceMap player_device_names;
		public:
			InputHandler() = default;
			InputHandler(EngineButtonMap&& button_name_to_value);

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

			// Simple forwarding overload for `InputDevices::process_event`.
			inline bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr)
			{
				return devices.process_event(e, opt_event_handler);
			}

			// Polls input on all device types.
			const InputDevices& poll(engine::EventHandler* opt_event_handler);
	};
}