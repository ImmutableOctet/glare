#include "mouse.hpp"

// SDL:
#include <sdl2/SDL_hints.h>
#include <sdl2/SDL_mouse.h>

namespace app::input
{
	Mouse::Mouse(bool locked)
	{
		if (locked)
		{
			lock();
		}
	}

	void Mouse::peek(State& state) const
	{
		auto buttons = SDL_GetRelativeMouseState(&state.x, &state.y);

		state.left   = (buttons & SDL_BUTTON(1));
		state.middle = (buttons & SDL_BUTTON(2));
		state.right  = (buttons & SDL_BUTTON(3));
	}

	bool Mouse::lock()
	{
		if (unlocked())
		{
			SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
			//SDL_CaptureMouse(SDL_TRUE);

			this->is_locked = true;

			return true;
		}

		return false;
	}

	bool Mouse::unlock()
	{
		if (locked())
		{
			SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "0", SDL_HINT_OVERRIDE);
			SDL_SetRelativeMouseMode(SDL_FALSE);
			//SDL_CaptureMouse(SDL_FALSE);

			this->is_locked = false;

			return true;
		}

		return false;
	}

	bool Mouse::set_lock(bool lock_state)
	{
		if (lock_state)
		{
			return lock();
		}

		return unlock();
	}
}