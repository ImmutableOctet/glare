#include "keyboard.hpp"
#include "keycodes.hpp"

#include "input_profile_impl.hpp"

#include <sdl2/SDL_keyboard.h>

namespace app::input
{
	Keyboard::Keyboard() {} // { peek(this->state); }

	void Keyboard::peek(Keyboard::State& state) const
	{
		int num_keys;

		auto* key_data = SDL_GetKeyboardState(&num_keys);

		state =
		{
			reinterpret_cast<decltype(KeyboardState::keys)>(key_data),
			static_cast<decltype(KeyboardState::num_keys)>(num_keys)
		};
	}

	std::string Keyboard::get_device_name() const
	{
		return std::string(peek_device_name());
	}

	std::string_view Keyboard::peek_device_name() const
	{
		return "Keyboard";
	}

	const KeyboardProfile* Keyboard::get_profile() const
	{
		if (device_profile)
		{
			return &(device_profile.value());
		}

		return nullptr;
	}

	// TODO: Look into the ability to manage multiple mouse profiles.
	const KeyboardProfile* Keyboard::load_profile(const ProfileMetadata& profile_metadata)
	{
		return input_profile_impl::load_profile
		(
			profile_metadata,
			this->device_profile,
			"keyboards",
			this->peek_device_name()
		);
	}
}