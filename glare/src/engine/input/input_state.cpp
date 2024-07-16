#include "input_state.hpp"

namespace engine
{
	InputState::ButtonStates::ButtonStates(ButtonsRaw bits) :
		bits(bits)
	{}

	bool InputState::ButtonStates::get_button(Button button) const
	{
		const auto button_raw = static_cast<ButtonsRaw>(button);

		return (bits & button_raw);
	}

	void InputState::ButtonStates::set_button(Button button, bool value)
	{
		const auto button_raw = static_cast<ButtonsRaw>(button);

		if (value)
		{
			bits |= button_raw;
		}
		else
		{
			bits &= ~button_raw;
		}
	}

	void InputState::ButtonStates::clear()
	{
		bits = {};
	}

	bool InputState::ButtonStates::any() const
	{
		return static_cast<bool>(bits);
	}
}