#include "keyboard.hpp"
//#include "keycodes.hpp"

#include <sdl2/SDL_keyboard.h>

namespace app
{
	namespace input
	{
		Keyboard::Keyboard() { peek(); }

		Keyboard::State Keyboard::peek()
		{
			int num_keys;

			auto* key_data = reinterpret_cast<const State::key_value_t*>(SDL_GetKeyboardState(&num_keys));

			this->num_keys = static_cast<decltype(this->num_keys)>(num_keys);

			return { key_data };
		}
	}
}