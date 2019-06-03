#pragma once

#include "types.hpp"

namespace app
{
	namespace input
	{
		class Keyboard
		{
			struct State : public DeviceState<256> {};

			Keyboard();

			State get_state();
		};
	}
}