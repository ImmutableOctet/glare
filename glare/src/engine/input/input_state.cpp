#include "input_state.hpp"

namespace engine
{
	bool InputState::ButtonStates::get_button(Button button) const
	{
		auto button_raw = static_cast<ButtonsRaw>(button);

		return (bits & button_raw);
	}

	void InputState::ButtonStates::set_button(Button button, bool value)
	{
		auto button_raw = static_cast<ButtonsRaw>(button);

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
		bits = 0;
	}
}