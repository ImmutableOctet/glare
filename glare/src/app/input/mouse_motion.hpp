#pragma once

//#include "types.hpp"
#include <types.hpp>

namespace app::input
{
	// `Mouse` equivalent to `GamepadAnalog`.
	enum class MouseMotion : std::uint8_t
	{
		// Standard relative mouse movement.
		Movement,      // XY

		// Encompasses both horizontal and vertical scroll capabilities.
		Scroll,        // XY

		// Currently unused:
		Gyroscope,     // XYZ
		Accelerometer, // XYZ

		// Indicates the starting index for dynamically defined 'analogs'.
		RuntimeAnalogOffset
	};
}