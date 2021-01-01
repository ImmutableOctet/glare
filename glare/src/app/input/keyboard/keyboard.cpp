#include "keyboard.hpp"
//#include "keycodes.hpp"

#include <sdl2/SDL_keyboard.h>

namespace app
{
	namespace input
	{
		// Keyboard:
		Keyboard::Keyboard() { peek(); }

		Keyboard::State Keyboard::peek()
		{
			int num_keys;

			auto* key_data = SDL_GetKeyboardState(&num_keys);

			return { reinterpret_cast<decltype(KeyboardState::keys)>(key_data), static_cast<decltype(KeyboardState::num_keys)>(num_keys) };
		}

		// KeyboardState:
		bool KeyboardState::get_key(int scan_code) const
		{
			if (!has_keys() || (scan_code < 0) || (scan_code > num_keys))
			{
				return false;
			}

			return (keys[scan_code] != 0);
		}

		bool KeyboardState::has_keys() const { return ((keys != nullptr) && (num_keys > 0)); }
	}
}