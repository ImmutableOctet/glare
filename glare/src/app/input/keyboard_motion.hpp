#pragma once

#include <types.hpp>

namespace app::input
{
	// Placeholder type; used as a stand-in for an actual analog enumeration.
	// NOTE: The possible values of this enum can be mapped at runtime via `Hat`s, etc.
	// `Keyboard` equivalent to `GamepadAnalog`, `MouseMotion`, etc.
	enum class KeyboardMotion : std::uint8_t
	{
		RuntimeAnalogOffset = 0,

		_Runtime_Reserved_0 = RuntimeAnalogOffset,
		_Runtime_Reserved_1,
		_Runtime_Reserved_2,
		_Runtime_Reserved_3,
		_Runtime_Reserved_4,
		_Runtime_Reserved_5,
		_Runtime_Reserved_6,
		_Runtime_Reserved_7,

		_Max

		//Touchpad,
		//Nub
	};
}