#pragma once

//#include <types.hpp>
//#include <math/types.hpp>

#include "mouse_buttons.hpp"

namespace app::input
{
	struct MouseState
	{
		// Relative motion on the X-axis.
		int x = 0;

		// Relative motion on the Y-axis.
		int y = 0;

		// Relative wheel motion on the X-axis.
		int wheel_x = 0;

		// Relative wheel motion on the Y-axis.
		int wheel_y = 0;

		// Button states:
		bool left    : 1 = false;
		bool middle  : 1 = false;
		bool right   : 1 = false;
		bool back    : 1 = false;
		bool forward : 1 = false;

		bool get_button(MouseButton button) const;
	};
}