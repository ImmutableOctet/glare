#include "keyboard.hpp"
#include "keycodes.hpp"

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
}