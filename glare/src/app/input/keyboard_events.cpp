#include "keyboard_events.hpp"

#include <math/math.hpp>

namespace app::input
{
	float OnKeyboardAnalogInput::angle() const
	{
		return math::direction_to_angle(value);
	}
}