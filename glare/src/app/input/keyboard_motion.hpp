#pragma once

#include <cstdint>

namespace app::input
{
	// Placeholder type; used as a stand-in for an actual analog enumeration.
	// NOTE: The possible values of this enum can be mapped at runtime via `Hat`s, etc.
	// `Keyboard` equivalent to `GamepadAnalog`, `MouseMotion`, etc.
	enum class KeyboardMotion : std::uint8_t
	{
		RuntimeAnalogOffset = 0,

		_runtime_reserved_0 = RuntimeAnalogOffset,
		_runtime_reserved_1,
		_runtime_reserved_2,
		_runtime_reserved_3,
		_runtime_reserved_4,
		_runtime_reserved_5,
		_runtime_reserved_6,
		_runtime_reserved_7,

		_max

		//Touchpad,
		//Nub
	};
}