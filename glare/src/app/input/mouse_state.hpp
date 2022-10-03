#pragma once

//#include <types.hpp>
//#include <math/types.hpp>

namespace app::input
{
	struct MouseState
	{
		int x = 0;
		int y = 0;

		bool left   : 1 = false;
		bool middle : 1 = false;
		bool right  : 1 = false;
	};
}