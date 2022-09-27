#pragma once

#include "types.hpp"

#include <math/types.hpp>

namespace app::input
{
	using GamepadDeviceIndex = std::uint8_t; // SDL_JoystickID;

	struct GamepadState
	{
		// 2D vector type used to store normalized analog/axis values.
		using Vector = math::Vector2D;

		// Integral type used to represent a button bitfield.
		using ButtonsRaw = std::uint16_t;

		// Maximum number of bits allocated to `Buttons::bits`.
		static constexpr std::size_t MAX_BUTTONS = (sizeof(ButtonsRaw) * 8); // 16;
		static constexpr ButtonsRaw DPAD_BIT_OFFSET = 10; // Starting at 0. (Bits 11-14)

		GamepadState();
		//GamepadState(const GamepadState&) = default;
		//GamepadState(GamepadState&&) noexcept = default;

		//~GamepadState();

		union Buttons
		{
			ButtonsRaw bits;

			// NOTE: Technically UB according to the C++ standard,
			// but supported on most platforms anyway:
			struct
			{
				// Button        // Bit
				bool A      : 1; // 1
				bool B      : 1; // 2
				bool X      : 1; // 3
				bool Y      : 1; // 4

				bool LB     : 1; // 5
				bool RB     : 1; // 6
				
				bool Select : 1; // 7
				bool Start  : 1; // 8

				bool L3     : 1; // 9
				bool R3     : 1; // 10

				// NOTE: Same layout as SDL for bitfield compatibility. (URDL)
				bool UP     : 1; // 11
				bool RIGHT  : 1; // 12
				bool DOWN   : 1; // 13
				bool LEFT   : 1; // 14

				// ...           // Remaining 2 bits reserved.
			};
		};

		Vector left_analog;
		Vector right_analog;
		Vector triggers;

		Buttons buttons;

		void update_buttons(ButtonsRaw buttons_encoded, bool value);
		
		void set_button(GamepadButtonID index, bool value);
		bool get_button(GamepadButtonID index) const;

		Vector dpad_direction() const;
		float dpad_angle() const;

		// Clears the values for the D-Pad fields in `buttons`.
		// This is usually called automatically via `update_dpad`.
		void clear_dpad();

		// Sets the values for the D-Pad fields in `buttons`.
		// 
		// `sdl_dpad_bits` is a bitfield whose first four bits are used for the directions of the D-Pad.
		// (Formated as URDL to be in line with SDL's joyhat events)
		// 
		// This differs from `update_buttons` which takes a standard bitfield formatted the same as `Buttons`.
		void update_dpad(std::uint8_t sdl_dpad_bits, bool clear_existing=true);

		// Indicates if no D-Pad directions are currently being pressed.
		bool no_directional_input() const; // hat_centered

		// Indicates whether a D-Pad directional input is active.
		bool has_directional_input() const;
	};
}