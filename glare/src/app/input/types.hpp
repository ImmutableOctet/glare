#pragma once

//#include <array>

#include <math/types.hpp>

#include <cstdint>
#include <unordered_map>
#include <string>

namespace app::input
{
	// Device instance identifier for physical `Mouse` and touch-related devices.
	using MouseDeviceIndex = std::uint8_t;
	
	// Underlying integral type used for `MouseButton` enumeration.
	using MouseButtonID = std::uint8_t;

	// Device instance identifier for physical `Keyboard` devices.
	using KeyboardDeviceIndex = std::uint8_t;

	// Underlying integral type used for `KeyboardButton` enumeration.
	using KeyboardButtonID = std::uint16_t; // std::uint32_t;

	// Smallest type that holds a key's held/released state.
	using KeyboardButtonValue = std::uint8_t;

	// Device instance identifier for `Gamepad` objects.
	using GamepadDeviceIndex = std::uint8_t; // SDL_JoystickID;

	// Minimal integral type used to represent individual (unencoded) buttons of a `Gamepad`.
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
	using EngineButtonsRaw = std::uint64_t; // std::uint32_t;

	// Raw integral type used to represent an enumeration type
	// for analog/floating-point inputs used by a game engine.
	using EngineAnalogsRaw = std::uint8_t; // std::uint16_t;

	// String-based name mappings used for loading configuration files. (e.g. JSON)
	using EngineButtonMap = std::unordered_map<std::string, EngineButtonsRaw>;

	// String-based name mappings used for loading configuration files. (e.g. JSON)
	using EngineAnalogMap = std::unordered_map<std::string, EngineAnalogsRaw>;

	// Maps string-based device names to player input identifiers.
	using PlayerDeviceMap = std::unordered_map<std::string, PlayerInputID>;

	// Event-type templates:
	
	// Base template type for device events that capture a snapshot of the input-state.
	// This type is not aliased, nor is it triggered directly.
	template <typename IndexType, typename StateType>
	struct InputStateEvent
	{
		// The device that triggered this input event.
		// NOTE: In the case of mice and keyboards, this is usually index 0.
		IndexType device_index;

		// A snapshot of the device's state at the time of this event.
		StateType state;
	};
}