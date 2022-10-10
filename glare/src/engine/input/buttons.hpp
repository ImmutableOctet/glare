#pragma once

#include <types.hpp>
#include <app/input/types.hpp>

namespace engine
{
	// Raw integral type used as a bitfield for button states.
	using EngineButtonsRaw = app::input::EngineButtonsRaw; // std::uint32_t; // std::uint64_t;
	using ButtonsRaw = EngineButtonsRaw;

	// Equivalent to `std::unordered_map<std::string, EngineButtonsRaw>`.
	using EngineButtonMap = app::input::EngineButtonMap;

	// TODO: Move this to a different header/file.
	// Enumeration of bits representing button states.
	enum class Button : ButtonsRaw
	{
		Jump         = (1 << 0), // Gamepad: A
		HeavyAttack  = (1 << 1), // Gamepad: B
		Interact     = (1 << 2), // Gamepad: X
		MediumAttack = (1 << 3), // Gamepad: Y

		Pause = (1 << 4), // Gamepad: Start

		// TODO: Change to a different bit.
		Shield = (1 << 5), // Gamepad: LT

		// TODO: Change to a different bit.
		FirstPerson = (1 << 6), // Gamepad: R3

		// Aliases:
		Slide = Shield,   // Gamepad: LT

		// Menu:
		MenuSelect = Jump,
		//MenuBack = ...,                   // Gamepad: B
		//MenuCycleForward = ...,           // Gamepad: RB
		//MenuSelectAlt = MenuCycleForward, // Gamepad: RB
		//MenuCycleBackward = ...,          // Gamepad: LB
		//MenuBackAlt = MenuCycleBackward,  // Gamepad: LB
	};

	using EngineButton = Button;

	// TODO: Move this to a different header/file.
	// Maps button names to their bitwise positional values/masks.
	// (Generated via `magic_enum`)
	void generate_button_map(EngineButtonMap& buttons);
}