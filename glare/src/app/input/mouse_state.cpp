#include "mouse_state.hpp"

namespace app::input
{
	bool MouseState::get_button(MouseButton button) const
	{
		using enum MouseButton;

		switch (button)
		{
			case Left:
				return left;
			case Middle:
				return middle;
			case Right:
				return right;
			case Back:
				return back;
			case Forward:
				return forward;
		}

		return false;
	}
}