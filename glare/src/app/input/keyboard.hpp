#pragma once

#include <app/input/types.hpp>

#include "keycodes.hpp"

namespace app::input
{
	class Keyboard : public InputDevice<KeyboardState>
	{
		public:
			Keyboard();

			virtual State peek() override;
	};
}