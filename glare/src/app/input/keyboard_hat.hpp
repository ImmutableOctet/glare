#pragma once

//#include "types.hpp"
#include "keyboard_buttons.hpp"
#include "hat.hpp"

namespace app::input
{
	// 4-directional Hat switch descriptor for `KeyboardProfile` objects.
	using KeyboardHat = Hat<KeyboardButton>;
}