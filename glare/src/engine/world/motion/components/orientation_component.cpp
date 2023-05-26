#include "orientation_component.hpp"

#include <engine/transform.hpp>

#include <math/math.hpp>

namespace engine
{
	OrientationComponent OrientationComponent::from_direction(const math::Vector& direction, float turn_speed)
	{
		auto component_out = OrientationComponent { {}, turn_speed };

		component_out.set_direction(direction);

		return component_out;
	}

	void OrientationComponent::set_direction(const math::Vector& direction)
	{
		this->orientation = Transform::quat_orientation({}, direction);
	}

	math::Vector OrientationComponent::get_direction() const
	{
		constexpr auto forward = math::Vector { 0.0f, 0.0f, -1.0f };

		return math::normalize(this->orientation * forward);
	}
}