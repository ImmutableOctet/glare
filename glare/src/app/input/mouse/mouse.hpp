#pragma once

#define _ENABLE_ATOMIC_ALIGNMENT_FIX

#include <app/input/types.hpp>
//#include <atomic>

namespace app
{
	namespace input
	{
		class Mouse : public InputDevice<MouseState>
		{
			public:
				Mouse(bool locked=false);

				virtual State peek() override;

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
}