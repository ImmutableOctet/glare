#pragma once

//#include "types.hpp"
#include <types.hpp>

#include <tuple>

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

	using MouseAnalogInputRaw = std::int32_t; // int;
	using MouseAnalogInput = std::tuple<MouseAnalogInputRaw, MouseAnalogInputRaw>;
}