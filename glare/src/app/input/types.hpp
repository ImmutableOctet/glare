#pragma once

//#include <array>

#include <types.hpp>

#include <math/types.hpp>

namespace app::input
{
	using GamepadDeviceIndex = std::uint8_t; // SDL_JoystickID;
	using GamepadButtonID = std::uint8_t;

	enum class GamepadAnalogInput : std::uint8_t
	{
		Left,
		Right,
		Triggers,
		DPad,
	};
}