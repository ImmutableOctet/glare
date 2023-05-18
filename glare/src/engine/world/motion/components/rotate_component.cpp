#include "rotate_component.hpp"

#include <engine/transform.hpp>

#include <math/math.hpp>

namespace engine
{
	RotateComponent RotateComponent::from_direction(const math::Vector& direction, float turn_speed, bool use_local_rotation)
	{
		auto component_out = RotateComponent { {}, turn_speed, use_local_rotation };

		component_out.set_direction(direction);

		return component_out;
	}

	math::Quaternion RotateComponent::get_next_basis(const Transform& tform, float delta) const
	{
		const auto tform_basis = (use_local_rotation)
			? tform.get_basis_q()
			: tform.get_local_basis_q()
		;

		return math::slerp(tform_basis, (tform_basis * relative_orientation), (turn_speed * delta));
	}

	void RotateComponent::set_direction(const math::Vector& direction)
	{
		this->relative_orientation = Transform::quat_orientation({}, direction); // orientation
		//this->relative_orientation = quaternion_from_orthogonal(...);
	}

	math::Vector RotateComponent::get_next_direction(Entity self, Registry& registry, float delta) const
	{
		auto tform = Transform(registry, self);

		return get_next_direction(tform, delta);
	}

	math::Vector RotateComponent::get_next_direction(const Transform& tform, float delta) const
	{
		constexpr auto forward = math::Vector { 0.0f, 0.0f, -1.0f };

		const auto next_basis = get_next_basis(tform, delta);

		const auto next_direction = math::normalize(next_basis * forward);

		return next_direction;
	}
}