#include "gamepad_state.hpp"

#include <math/joyhat.hpp>

#include <sdl2/SDL_joystick.h>

namespace app::input
{
	static constexpr std::uint8_t SDL_DPAD_MASK = (SDL_HAT_UP | SDL_HAT_RIGHT | SDL_HAT_DOWN | SDL_HAT_LEFT); // (2 << 3);

	GamepadState::GamepadState()
		: buttons { .bits = 0 }
	{}

	//GamepadState::~GamepadState() {}

	void GamepadState::update_buttons(GamepadButtonsRaw buttons_encoded, bool value)
	{
		if (value)
		{
			buttons.bits |= buttons_encoded;
		}
		else
		{
			buttons.bits &= ~buttons_encoded;
		}
	}

	void GamepadState::set_button(GamepadButtonID index, bool value)
	{
		update_buttons((static_cast<GamepadButtonsRaw>(1) << static_cast<GamepadButtonsRaw>(index)), value);
	}

	bool GamepadState::get_button(GamepadButtonID index) const
	{
		return (buttons.bits & (static_cast<GamepadButtonsRaw>(1) << static_cast<GamepadButtonsRaw>(index)));
	}

	GamepadState::ButtonBit GamepadState::get_dpad_button(DPad_Direction direction) const
	{
		switch (direction)
		{
			case DPad_Direction::Up:
				return ButtonBit::UP;
			case DPad_Direction::Right:
				return ButtonBit::RIGHT;
			case DPad_Direction::Down:
				return ButtonBit::DOWN;
			case DPad_Direction::Left:
				return ButtonBit::LEFT;
		}

		// This should not happen; invalid directional input.
		assert(false);

		// If nothing else, return the `UP` button.
		return ButtonBit::UP;
	}

	bool GamepadState::get_dpad(DPad_Direction direction) const
	{
		return get_button(get_dpad_button(direction));
	}

	std::optional<GamepadState::Vector> GamepadState::get_analog(GamepadAnalog analog) const
	{
		using enum GamepadAnalog;

		switch (analog)
		{
			case Left:
				return left_analog;
			case Right:
				return right_analog;
			case Triggers:
				return triggers;
			case DPad:
				return dpad_direction();
		}

		return std::nullopt;
	}

	GamepadState::Vector GamepadState::dpad_direction() const
	{
		return math::joyhat(buttons.UP, buttons.DOWN, buttons.LEFT, buttons.RIGHT);
	}

	float GamepadState::dpad_angle() const
	{
		return math::joyhat_angle(buttons.UP, buttons.DOWN, buttons.LEFT, buttons.RIGHT);
	}

	void GamepadState::clear_dpad()
	{
		constexpr auto dpad_apply_mask = (static_cast<GamepadButtonsRaw>(SDL_DPAD_MASK) << DPAD_BIT_OFFSET);

		// Clear the current D-Pad state.
		update_buttons(dpad_apply_mask, false);
	}

	void GamepadState::update_dpad(std::uint8_t sdl_dpad_bits, bool clear_existing)
	{
		if (clear_existing)
		{
			clear_dpad();
		}

		update_buttons((static_cast<GamepadButtonsRaw>(sdl_dpad_bits & SDL_DPAD_MASK) << DPAD_BIT_OFFSET), true);
	}

	bool GamepadState::no_directional_input() const
	{
		// NOTE: This could be better optimized using bitmasks.
		return ((!buttons.UP) && (!buttons.RIGHT) && (!buttons.DOWN) && (!buttons.LEFT));
	}

	bool GamepadState::has_directional_input() const
	{
		return !no_directional_input();
	}
}