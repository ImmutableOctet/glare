#pragma once

#include "types.hpp"

namespace app::input
{
	enum class MouseButton : MouseButtonID
	{
		Left    = 1,
		Middle  = 2,
		Right   = 3,

		Back    = 4,
		Forward = 5,

		// Alternate names:
		X1 = Back,
		X2 = Forward,

		M1 = Left,
		M2 = Right,
		M3 = Middle,
		M4 = X1,
		M5 = X2,
	};
}