#pragma once

#include "types.hpp"
#include "gamepad_buttons.hpp"
#include "gamepad_analog.hpp"

#include <math/types.hpp>

#include <util/magic_enum.hpp>
#include <optional>

namespace app::input
{
	using GamepadDeviceIndex = std::uint8_t; // SDL_JoystickID;

	struct GamepadState
	{
		// 2D vector type used to store normalized analog/axis values.
		using Vector = math::Vector2D;

		using ButtonBit = GamepadButtonBits;

		// Maximum number of bits allocated to `Buttons::bits`.
		static constexpr std::size_t MAX_BUTTONS = (sizeof(GamepadButtonsRaw) * 8); // 16;
		static constexpr GamepadButtonsRaw DPAD_BIT_OFFSET = 10; // Starting at 0. (Bits 11-14)

		// Enumerates values in the underlying button enum-type.
		template <typename Callback>
		static void enumerate_button_sequence(Callback&& callback)
		{
			magic_enum::enum_for_each<ButtonBit>
			(
				[&callback](auto button)
				{
					callback(button);
				}
			);
		}

		GamepadState();
		//GamepadState(const GamepadState&) = default;
		//GamepadState(GamepadState&&) noexcept = default;

		//~GamepadState();

		union Buttons
		{
			GamepadButtonsRaw bits;

			// NOTE: This should follow the same layout as `GamepadButtonBit`.
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

		void update_buttons(GamepadButtonsRaw buttons_encoded, bool value);
		
		void set_button(GamepadButtonID index, bool value);
		bool get_button(GamepadButtonID index) const;

		std::optional<Vector> get_analog(GamepadAnalog analog) const;

		// Type-safe version of `GamepadButtonID` overload; see `GamepadButtonBit`.
		inline void set_button(ButtonBit button_index_bit, bool value)
		{
			set_button(static_cast<GamepadButtonID>(button_index_bit), value);
		}

		// Type-safe version of `GamepadButtonID` overload; see `GamepadButtonBit`.
		inline bool get_button(ButtonBit button_index_bit) const
		{
			return get_button(static_cast<GamepadButtonID>(button_index_bit));
		}

		// Retrieves the current value for each button.
		// 
		// `callback` is a callable taking in a `GamepadButtonBits` value,
		// and a boolean value indicating the button's state.
		template <typename Callback>
		inline void inspect_buttons(Callback&& callback) const
		{
			enumerate_button_sequence([this, &callback](const auto& button)
			{
				const auto value = this->get_button(button);

				callback(button, value);
			});
		}

		template <typename Callback>
		inline int on_button_value(Callback&& callback, bool target_value) const
		{
			int buttons_found = 0;

			inspect_buttons([&callback, target_value, &buttons_found](const auto& button, bool value)
			{
				if (value == target_value)
				{
					callback(button);

					buttons_found++;
				}
			});

			return buttons_found;
		}

		template <typename Callback>
		inline int on_button_active(Callback&& callback) const
		{
			return on_button_value(callback, true);
		}

		template <typename Callback>
		inline int on_button_inactive(Callback&& callback) const
		{
			return on_button_value(callback, false);
		}

		template <typename Callback>
		inline int on_button_change(const GamepadState& prev_state, Callback&& callback) const
		{
			// Check if the `bits` bitfield is exactly the same:
			if (this->buttons.bits == prev_state.buttons.bits)
			{
				// No buttons have changed, exit early.
				return 0;
			}

			int buttons_changed = 0;

			inspect_buttons([&prev_state, &callback, &buttons_changed](const auto& button, const auto& value)
			{
				const auto prev_value = prev_state.get_button(button);

				if (value != prev_value)
				{
					callback(button, value); // , prev_value

					buttons_changed++;
				}
			});

			return buttons_changed;
		}

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