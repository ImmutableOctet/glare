#pragma once

//#include <array>

#include <types.hpp>

#include <math/types.hpp>

#include <unordered_map>
#include <string>

namespace app::input
{
	using GamepadDeviceIndex = std::uint8_t; // SDL_JoystickID;
	using GamepadButtonID = std::uint8_t;

	// Integral type used to represent a gamepad's button bitfield. (see `GamepadState::Buttons`)
	using GamepadButtonsRaw = std::uint16_t;

	// A 16-bit identifier used to map players to input devices.
	// (Usually handled as an incremental number)
	// 
	// NOTE: This acts as a proxy for `engine::PlayerIndex` -- These two type
	// aliases are intended to represent the same underlying integral type.
	using PlayerInputID = std::uint16_t;

	//using MouseButtonsRaw = std::uint8_t;

	// Raw integral type used to represent a bitfield
	// of button states used by a game engine.
	using EngineButtonsRaw = std::uint32_t; // std::uint64_t;

	// Raw integral type used to represent an enumeration type
	// for analog/floating-point inputs used by a game engine.
	using EngineAnalogsRaw = std::uint8_t; // std::uint16_t;

	// String-based name mappings used for loading configuration files. (e.g. JSON)
	using EngineButtonMap = std::unordered_map<std::string, EngineButtonsRaw>;

	// String-based name mappings used for loading configuration files. (e.g. JSON)
	using EngineAnalogMap = std::unordered_map<std::string, EngineAnalogsRaw>;

	// Maps string-based device names to player input identifiers.
	using PlayerDeviceMap = std::unordered_map<std::string, PlayerInputID>;
}