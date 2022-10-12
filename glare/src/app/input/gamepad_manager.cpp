#include "gamepad_manager.hpp"
#include "events.hpp"
#include "profile_metadata.hpp"

#include <sdl2/SDL_events.h>
#include <sdl2/SDL_joystick.h>
#include <sdl2/SDL_hints.h>

#include <util/log.hpp>
#include <util/json.hpp>
#include <util/format.hpp>

#include <stdexcept>

namespace app::input
{
	GamepadManager::GamepadManager(bool enable_events, bool background_events)
	{
		// Reserve for the standard number of gamepads (4).
		gamepads.reserve(4); // gamepads.resize(4);

		if (enable_events)
		{
			set_background_input(background_events);

			SDL_JoystickEventState(SDL_ENABLE);
		}
	}

	GamepadManager::GamepadManager(const ProfileMetadata& profile_metadata, bool enable_events, bool background_events)
		: GamepadManager(enable_events, background_events)
	{
		load_profiles(profile_metadata);
	}

	void GamepadManager::load_profiles(const ProfileMetadata& profile_metadata)
	{
		auto data = util::load_json(profile_metadata.path);

		// Load profiles:
		if (const auto profile_entries_it = data.find("profiles"); profile_entries_it != data.end())
		{
			const auto& profile_entries = *profile_entries_it;

			for (const auto& proxy : profile_entries.items())
			{
				const auto& profile_entry = proxy.value();
				const auto  profile_name  = profile_entry["name"].get<std::string>();

				profiles[profile_name] = GamepadProfile(profile_metadata, profile_entry);
				//profiles.emplace(profile_name, profile_metadata, profile_entry);
			}
		}

		// Load device-to-profile mappings:
		if (const auto gamepads_it = data.find("gamepads"); gamepads_it != data.end())
		{
			const auto& gamepads = *gamepads_it;

			for (const auto& proxy : gamepads.items())
			{
				const auto& device_entry = proxy.value();

				auto device_name  = device_entry["device_name"].get<std::string>();
				auto profile_name = device_entry["profile"].get<std::string>();

				auto result = map_device_to_profile(device_name, profile_name);

				if (!result)
				{
					throw std::runtime_error(format("Unable to find gamepad profile: \"{}\"", profile_name));
				}
			}
		}

		// Load device-to-player mappings:
		if (const auto players_it = data.find("players"); players_it != data.end())
		{
			const auto& players = *players_it;

			for (const auto& proxy : players.items())
			{
				const auto& player_entry = proxy.value();

				auto device_name  = player_entry["device_name"].get<std::string>();
				auto player_index = player_entry["player_index"].get<PlayerInputID>();

				profile_metadata.player_mappings_out[device_name] = player_index;
			}
		}
	}

	void GamepadManager::poll(entt::dispatcher* opt_event_handler)
	{
		for (auto& gamepad : gamepads)
		{
			auto* profile = get_profile(gamepad);

			gamepad.poll(profile, opt_event_handler);
		}
	}

	bool GamepadManager::process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler)
	{
		std::optional<GamepadDeviceIndex> gamepad_id;

		switch (e.type)
		{
			case SDL_JOYDEVICEADDED:
				gamepad_id = static_cast<GamepadDeviceIndex>(e.jdevice.which);

				on_gamepad_connected(*gamepad_id);

				if (opt_event_handler)
				{
					opt_event_handler->enqueue<OnGamepadConnected>(*gamepad_id);
				}

				// No need to pass the SDL event on, since we
				// already generated a `OnGamepadConnected` event.
				return true;

			case SDL_JOYDEVICEREMOVED:
				gamepad_id = static_cast<GamepadDeviceIndex>(e.jdevice.which);

				on_gamepad_disconnected(*gamepad_id);

				if (opt_event_handler)
				{
					opt_event_handler->enqueue<OnGamepadDisconnected>(*gamepad_id);
				}

				// No need to pass the event on, since we're disconnecting
				// the gamepad + generating `OnGamepadDisconnected`.
				return true; // false;

			case SDL_JOYBUTTONUP:
			case SDL_JOYBUTTONDOWN:
				gamepad_id = static_cast<GamepadDeviceIndex>(e.jbutton.which);

				break;

			case SDL_JOYAXISMOTION:
				gamepad_id = static_cast<GamepadDeviceIndex>(e.jaxis.which);

				break;
			case SDL_JOYHATMOTION:
				gamepad_id = static_cast<GamepadDeviceIndex>(e.jhat.which);

				break;
			case SDL_JOYBALLMOTION:
				gamepad_id = static_cast<GamepadDeviceIndex>(e.jball.which);

				break;
		}

		if (gamepad_id)
		{
			auto& gamepad = get_gamepad(*gamepad_id);

			return gamepad.process_event(e, opt_event_handler);
		}

		return false;
	}

	Gamepad& GamepadManager::get_gamepad(GamepadDeviceIndex index) const
	{
		auto new_container_size = (static_cast<std::size_t>(index) + 1);

		if (new_container_size > gamepads.size())
		{
			gamepads.resize(new_container_size);
		}

		auto& gamepad = gamepads[index];

		bool already_open = gamepad.is_open();

		if (!already_open)
		{
			bool is_open = gamepad.open(index, false);

			if (is_open)
			{
				const auto* profile = get_profile(gamepad.get_device_name());

				if (profile)
				{
					apply_profile(gamepad, *profile);
				}
			}
		}

		return gamepad;
	}

	const GamepadProfile* GamepadManager::get_profile(const std::string& device_name) const
	{
		return get_profile(std::string_view { device_name });
	}

	const GamepadProfile* GamepadManager::get_profile(std::string_view device_name) const
	{
		if (device_name.empty())
		{
			return {};
		}

		const auto it = device_profile_map.find(device_name);

		if (it != device_profile_map.end())
		{
			return it->second;
		}

		return {};
	}

	const GamepadProfile* GamepadManager::get_profile(GamepadDeviceIndex index) const
	{
		return get_profile(get_gamepad(index));
	}

	const GamepadProfile* GamepadManager::get_profile(const Gamepad& gamepad) const
	{
		return get_profile(gamepad.get_device_name_as_view());
	}

	void GamepadManager::apply_profile(Gamepad& gamepad, const GamepadProfile& profile) const
	{
		gamepad.apply_profile(profile);
	}

	void GamepadManager::set_background_input(bool enabled, bool force)
	{
		SDL_SetHintWithPriority
		(
			SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,
			
			(enabled)
			? "1"
			: "0",

			(force)
			? SDL_HintPriority::SDL_HINT_OVERRIDE
			: SDL_HintPriority::SDL_HINT_DEFAULT
		);
	}

	int GamepadManager::count_open_gamepads() const
	{
		//return enumerate_gamepads([]() { return true });

		int gamepad_count = 0; // GamepadDeviceIndex

		for (const auto& gamepad : gamepads)
		{
			gamepad_count += static_cast<int>(gamepad.is_open());
		}

		return gamepad_count;
	}

	GamepadProfile* GamepadManager::get_profile(std::string_view device_name)
	{
		if (device_name.empty())
		{
			return {};
		}

		auto it = device_profile_map.find(device_name);

		if (it != device_profile_map.end())
		{
			return it->second;
		}

		return {};
	}

	GamepadProfile* GamepadManager::get_profile(const Gamepad& gamepad)
	{
		return get_profile(gamepad.get_device_name_as_view());
	}

	const GamepadProfile* GamepadManager::map_device_to_profile(const std::string& device_name, const std::string& profile_name)
	{
		auto profile_it = profiles.find(profile_name);

		if (profile_it != profiles.end())
		{
			auto* profile = &profile_it->second;

			if (map_device_to_profile(device_name, profile))
			{
				return profile;
			}
		}

		return nullptr;
	}

	bool GamepadManager::map_device_to_profile(const std::string& device_name, GamepadProfile* profile)
	{
		auto device_it = device_profile_map.find(device_name);

		if (device_it != device_profile_map.end())
		{
			return false;
		}

		device_profile_map[device_name] = profile;

		return true;
	}

	bool GamepadManager::unmap_device_profile(const std::string& device_name)
	{
		auto device_it = device_profile_map.find(device_name);

		if (device_it != device_profile_map.end())
		{
			device_profile_map.erase(device_it);

			// Device mapping successfully removed.
			return true;
		}

		// Couldn't find a mapping for `device_name`.
		return false;
	}

	void GamepadManager::on_gamepad_connected(GamepadDeviceIndex device_index)
	{
		// Establish the gamepad instance, but don't use it.
		get_gamepad(device_index);
	}

	void GamepadManager::on_gamepad_disconnected(GamepadDeviceIndex device_index)
	{
		// Retrieve the current gamepad instance for `device_index`.
		auto& gamepad = get_gamepad(device_index);

		// Close the gamepad handle.
		gamepad.close();
	}
}