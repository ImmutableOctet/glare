#include "events.hpp"

//#include <math/joyhat.hpp>
#include <math/math.hpp>

namespace app::input
{
	float OnGamepadAnalogInput::angle() const
	{
		switch (analog)
		{
			case GamepadAnalogInput::Triggers:
				return 0.0f;
		}

		return math::direction_to_angle(value);
	}
}