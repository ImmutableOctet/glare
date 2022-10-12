#pragma once

#include "types.hpp"
#include "gamepad.hpp"
#include "gamepad_profile.hpp"

//#include "profile_metadata.hpp"

//#include <engine/types.hpp>
#include <entt/signal/dispatcher.hpp>

#include <vector>
#include <filesystem>
#include <string>
#include <string_view>
#include <optional>
#include <functional>

#include <map>
//#include <unordered_map>

namespace app::input
{
	struct ProfileMetadata;

	class GamepadManager
	{
		public:
			GamepadManager(bool enable_events=true, bool background_events=true);
			GamepadManager(const ProfileMetadata& profile_metadata, bool enable_events=true, bool background_events=true);

			// Loads gamepad profile data from the `profile_metadata` specified.
			// NOTE: Although `profile_metadata` is a const-ref here, its contents may specify output parameters.
			void load_profiles(const ProfileMetadata& profile_metadata);

			void poll(entt::dispatcher* opt_event_handler);
			bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr);

			// NOTE: `Gamepad` handles are not guaranteed to remain valid.
			// Use this method to temporarily retrieve a `Gamepad` object for a specific device-index.
			Gamepad& get_gamepad(GamepadDeviceIndex index) const;
			const GamepadProfile* get_profile(const std::string& device_name) const;
			const GamepadProfile* get_profile(std::string_view device_name) const;
			const GamepadProfile* get_profile(GamepadDeviceIndex index) const;
			const GamepadProfile* get_profile(const Gamepad& gamepad) const;

			void apply_profile(Gamepad& gamepad, const GamepadProfile& profile) const;

			void set_background_input(bool enabled, bool force=false);

			// Enumerates 'open' gamepad devices, executing `callback` on each one.
			// If `callback` returns true, enumeration continues.
			// If `false` is returned, this function exits immediately.
			// The value returned by this function indicates the number of open gamepads.
			template <typename Callback>
			inline int enumerate_gamepads(Callback&& callback) const // GamepadDeviceIndex
			{
				int gamepad_count = 0;

				for (auto& gamepad : gamepads)
				{
					if (!gamepad.is_open())
					{
						continue;
					}

					auto result = callback(gamepad);

					// Check against `is_open` again, in case the user closed it.
					if (gamepad.is_open())
					{
						gamepad_count++;
					}

					if (!result)
					{
						break;
					}
				}

				return gamepad_count;
			}

			int count_open_gamepads() const; // GamepadDeviceIndex
		protected:
			// Used internally by `poll`.
			GamepadProfile* get_profile(std::string_view device_name);

			// Used internally by `poll`.
			GamepadProfile* get_profile(const Gamepad& gamepad);

			// Maps a device (by name) to a profile (by name).
			// This returns `nullptr` if a profile named `profile_name` could not be found.
			const GamepadProfile* map_device_to_profile(const std::string& device_name, const std::string& profile_name);

			// Maps devices named `device_name` to `profile`.
			// This returns false if a mapping already exists.
			bool map_device_to_profile(const std::string& device_name, GamepadProfile* profile);

			// Removes existing profile-mapping for `device_name`.
			// If no mapping currently exists, this will return false.
			bool unmap_device_profile(const std::string& device_name);

			void on_gamepad_connected(GamepadDeviceIndex device_index);
			void on_gamepad_disconnected(GamepadDeviceIndex device_index);

			// Map of gamepad profiles by name. (Usually loaded via JSON; see `load_profiles`)
			std::map<std::string, GamepadProfile, std::less<>> profiles; // unordered_map // std::hash<std::string>

			// Maps device names to corresponding profile entries in `profiles`.
			std::map<std::string, GamepadProfile*, std::less<>> device_profile_map; // unordered_map // std::hash<std::string> // const GamepadProfile*

			// Container of gamepad instances. Indices generally correspond to their respective device IDs.
			mutable std::vector<Gamepad> gamepads;
	};
}