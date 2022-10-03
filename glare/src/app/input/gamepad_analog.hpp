#pragma once

//#include "types.hpp"
#include <types.hpp>

namespace app::input
{
	enum class GamepadAnalog : std::uint8_t
	{
		Left,
		Right,
		Triggers,
		DPad,
	};
}