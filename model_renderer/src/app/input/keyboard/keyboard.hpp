#pragma once

#include <app/input/types.hpp>

#include "keycodes.hpp"

namespace app
{
	namespace input
	{
		class Keyboard : public InputDevice<KeyboardState>
		{
			public:
				Keyboard();

				virtual State peek() override;
			protected:
				std::uint32_t num_keys = 0;
		};
	}
}