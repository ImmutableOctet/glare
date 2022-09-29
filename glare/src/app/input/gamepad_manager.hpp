#pragma once

#include "gamepad.hpp"
#include "gamepad_profile.hpp"

//#include <engine/types.hpp>
#include <entt/signal/dispatcher.hpp>

#include <vector>
#include <filesystem>
#include <string>
#include <optional>

//#include <map>
#include <unordered_map>

namespace app::input
{
	class GamepadManager
	{
		public:
			GamepadManager(std::optional<std::filesystem::path> profile_path="config/input/gamepads.json", bool enable_events=true, bool background_events=true);

			void load_profiles(const std::filesystem::path& path);

			void poll(entt::dispatcher& event_handler);
			bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr);

			// NOTE: `Gamepad` handles are not guaranteed to remain valid.
			// Use this method to temporarily retrieve a `Gamepad` object for a specific device-index.
			Gamepad& get_gamepad(GamepadDeviceIndex index);
			const GamepadProfile* get_profile(const std::string& device_name) const;

			void apply_profile(Gamepad& gamepad, const GamepadProfile& profile);

			void set_background_input(bool enabled, bool force=false);
		protected:
			// Maps a device (by name) to a profile (by name).
			// This returns `nullptr` if a profile named `profile_name` could not be found.
			const GamepadProfile* map_device_to_profile(const std::string& device_name, const std::string& profile_name);

			// Maps devices named `device_name` to `profile`.
			// This returns false if a mapping already exists.
			bool map_device_to_profile(const std::string& device_name, const GamepadProfile* profile);

			// Removes existing profile-mapping for `device_name`.
			// If no mapping currently exists, this will return false.
			bool unmap_device_profile(const std::string& device_name);

			void on_gamepad_connected(GamepadDeviceIndex device_index);
			void on_gamepad_disconnected(GamepadDeviceIndex device_index);

			// Map of gamepad profiles by name. (Usually loaded via JSON; see `load_profiles`)
			std::unordered_map<std::string, GamepadProfile> profiles;

			// Maps device names to corresponding profile entries in `profiles`.
			std::unordered_map<std::string, const GamepadProfile*> device_profile_map;

			// Container of gamepad instances. Indices generally correspond to their respective device IDs.
			std::vector<Gamepad> gamepads;
	};
}