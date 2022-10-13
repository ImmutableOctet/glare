#include "virtual_button.hpp"

//#include <magic_enum/magic_enum.hpp>

namespace app::input
{
	bool VirtualButton::is_down(const math::Vector2D& values) const
	{
		//using namespace magic_enum::bitwise_operators;

		if ((analog_axis & Axis::X))
		{
			return is_down(values.x);
		}

		if ((analog_axis & Axis::Y))
		{
			return is_down(values.y);
		}

		return false;
	}

	bool VirtualButton::is_down(const math::Vector3D& values) const
	{
		//using namespace magic_enum::bitwise_operators;

		if ((analog_axis & Axis::X))
		{
			return is_down(values.x);
		}

		if ((analog_axis & Axis::Y))
		{
			return is_down(values.y);
		}

		if ((analog_axis & Axis::Z))
		{
			return is_down(values.z);
		}

		return false;
	}

	bool VirtualButton::is_down(float value) const
	{
		switch (comparison)
		{
			case ComparisonMethod::Both:
				return (math::abs(value) >= math::abs(threshold)); // >

			case ComparisonMethod::Greater:
				return (value >= threshold); // >

			case ComparisonMethod::Lesser:
				return (value <= threshold); // <
		}

		return false;
	}
}