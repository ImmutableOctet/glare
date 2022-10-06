#pragma once

#include <types.hpp>

namespace app::input
{
	// Placeholder type.
	// `Keyboard` equivalent to `GamepadAnalog`, `MouseMotion`, etc.
	enum class KeyboardMotion : std::uint8_t
	{
		_Reserved,

		//Touchpad,
		//Nub
	};
}