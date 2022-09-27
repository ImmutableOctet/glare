#pragma once

#include "gamepad.hpp"
#include "gamepad_profile.hpp"

//#include <engine/types.hpp>
#include <entt/signal/dispatcher.hpp>

#include <vector>
#include <filesystem>
#include <string>
#include <optional>

namespace app::input
{
	class GamepadManager
	{
		public:
			GamepadManager(const std::filesystem::path& profile_root_path="assets/gamepads", bool enable_events=true);

			void poll(entt::dispatcher& event_handler);
			bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr);

			// NOTE: `Gamepad` handles are not guaranteed to remain valid.
			// Use this method to temporarily retrieve a `Gamepad` object for a specific device-index.
			Gamepad& get_gamepad(GamepadDeviceIndex index);
			std::optional<GamepadProfile> get_profile(const std::string& device_name) const;

			void apply_profile(Gamepad& gamepad, const GamepadProfile& profile);
		protected:
			void on_gamepad_connected(GamepadDeviceIndex device_index);
			void on_gamepad_disconnected(GamepadDeviceIndex device_index);

			std::filesystem::path profile_root_path;
			std::vector<Gamepad> gamepads;
	};
}