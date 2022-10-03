#pragma once

#define _ENABLE_ATOMIC_ALIGNMENT_FIX

#include "types.hpp"
#include "input_device.hpp"
#include "mouse_state.hpp"

//#include <atomic>

namespace app::input
{
	class Mouse : public InputDevice<MouseState>
	{
		public:
			Mouse(bool locked=false);

			virtual void peek(State& state) const override;

			//bool process_event(const SDL_Event& e, entt::dispatcher* opt_event_handler=nullptr) override { return false; }

			bool lock();
			bool unlock();

			bool set_lock(bool lock_state);

			inline bool toggle_lock() { return set_lock(!locked()); }

			inline bool locked() const   { return is_locked; }
			inline bool unlocked() const { return !locked(); }
		private:
			bool is_locked = false;
	};
}