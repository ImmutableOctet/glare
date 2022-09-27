#include "gamepad_manager.hpp"
#include "events.hpp"

#include <sdl2/SDL_events.h>
#include <sdl2/SDL_joystick.h>
#include <sdl2/SDL_hints.h>

#include <util/log.hpp>
#include <util/json.hpp>

namespace app::input
{
	GamepadManager::GamepadManager(const std::filesystem::path& profile_root_path, bool enable_events, bool background_events)
		: profile_root_path(profile_root_path)
	{
		// Reserve for the standard number of gamepads (4).
		gamepads.reserve(4); // gamepads.resize(4);

		if (enable_events)
		{
			set_background_input(background_events);

			SDL_JoystickEventState(SDL_ENABLE);
		}
	}

	void GamepadManager::poll(entt::dispatcher& event_handler)
	{
		for (auto& gamepad : gamepads)
		{
			gamepad.poll(event_handler);
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

				// No need to pass the SDL event on, since we already generated a `OnGamepadConnected` event.
				return true;

			case SDL_JOYDEVICEREMOVED:
				gamepad_id = static_cast<GamepadDeviceIndex>(e.jdevice.which);

				on_gamepad_disconnected(*gamepad_id);

				if (opt_event_handler)
				{
					opt_event_handler->enqueue<OnGamepadDisconnected>(*gamepad_id);
				}

				// No need to pass the event on, since we're disconnecting the gamepad + generating `OnGamepadDisconnected`.
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

	Gamepad& GamepadManager::get_gamepad(GamepadDeviceIndex index)
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
				auto profile = get_profile(gamepad.get_device_name());

				if (profile)
				{
					apply_profile(gamepad, *profile);
				}
			}
		}

		return gamepad;
	}

	std::optional<GamepadProfile> GamepadManager::get_profile(const std::string& device_name) const
	{
		if (device_name.empty())
		{
			return std::nullopt;
		}

		auto path = (profile_root_path / (device_name + ".json"));

		print("Loading gamepad profile from: \"{}\"...", path.string());
		
		return GamepadProfile(util::load_json(path));
	}

	void GamepadManager::apply_profile(Gamepad& gamepad, const GamepadProfile& profile)
	{
		gamepad.set_deadzone(profile.deadzone);
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