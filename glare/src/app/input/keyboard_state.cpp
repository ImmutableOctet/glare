#include "keyboard_state.hpp"
//#include "keycodes.hpp"

//#include <sdl2/SDL_keyboard.h>

namespace app::input
{
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