#include <math/math.hpp>

#include "analog.hpp"

namespace game
{
	bool InputAnalogStates::has_analog_input(Analog analog, float minimum_threshold) const
	{
		return (math::length(get_analog(analog)) > minimum_threshold);
	}

	float InputAnalogStates::angle_of(const DirectionVector& analog) const
	{
		return math::direction_to_angle(analog);
	}

	float InputAnalogStates::angle_of(Analog analog) const
	{
		return angle_of(get_analog(analog));
	}

	float InputAnalogStates::movement_angle() const
	{
		return angle_of(movement);
	}

	float InputAnalogStates::camera_angle() const
	{
		return angle_of(camera);
	}

	float InputAnalogStates::menu_select_angle() const
	{
		return angle_of(menu_select);
	}

	float InputAnalogStates::orientation_angle() const
	{
		return angle_of(orientation);
	}
}