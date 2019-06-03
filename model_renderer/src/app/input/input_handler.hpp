#pragma once

//#include "types.hpp"
#include "devices.hpp"

namespace app
{
	namespace input
	{
		class InputHandler
		{
			protected:
				// Live device state.
				InputState state;
			public:
				InputState poll();

				//SDL_CaptureMouse
		};
	}
}