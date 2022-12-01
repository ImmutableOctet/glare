#include "target_behavior.hpp"

#include <engine/world/world.hpp>
#include <engine/transform.hpp>
#include <util/string.hpp>

namespace engine
{
	TargetBehavior::Mode TargetBehavior::resolve_mode(const std::string& mode_name)
	{
		const auto m = util::lowercase(mode_name);
		
		if (m == "lookat")
		{
			return Mode::LookAt;
		}

		if (m == "lookat_immediate")
		{
			return Mode::LookAt_Immediate;
		}

		if (m == "yaw")
		{
			return Mode::Yaw;
		}

		if (m == "yaw_immediate")
		{
			return Mode::Yaw_Immediate;
		}

		return Mode::Default;
	}

	void TargetBehavior::on_update(World& world, float delta)
	{
		auto& registry = world.get_registry();

		registry.view<TargetBehavior>().each([&](auto entity, auto& target_comp) // TransformComponent, ...
		{
			if (target_comp.target == null)
			{
				return;
			}

			auto transform = world.get_transform(entity);

			target_comp.apply(world, entity, transform, delta);
		});
	}

	math::Quaternion TargetBehavior::look_at(Transform& transform, Transform& target_transform, float delta)
	{
		//auto dest_m = Transform::orientation(transform.get_position(), target_transform.get_position());
		//auto dest_q = glm::quat_cast(dest_m);

		auto dest_q = Transform::quat_orientation(target_transform.get_position(), transform.get_position());
		//auto dest_q = Transform::quat_orientation(target_transform.get_local_position(), transform.get_local_position());
		auto src_q = glm::quat_cast(transform.get_basis());

		auto q = math::slerp(src_q, dest_q, (interpolation * delta));

		transform.set_basis_q(q);
		//transform.set_local_basis_q(q);

		return q;
	}

	math::RotationMatrix TargetBehavior::look_at_immediate(Transform& transform, Transform& target_transform)
	{
		return transform.look_at(target_transform);
	}

	float TargetBehavior::look_at_yaw(Transform& transform, Transform& target_transform, float delta)
	{
		auto src_dir = transform.get_direction_vector();
		auto dest_dir = (target_transform.get_position() - transform.get_position());

		auto yaw = math::nlerp_radians(src_dir, dest_dir, (interpolation * delta));
		//auto yaw = math::direction_to_yaw(src_dir);

		transform.set_ry(yaw);

		return yaw;
	}

	float TargetBehavior::look_at_yaw_immediate(Transform& transform, Transform& target_transform)
	{
		auto dir = (target_transform.get_position() - transform.get_position());
		auto yaw = math::direction_to_yaw(dir);

		transform.set_ry(yaw);

		return yaw;
	}

	void TargetBehavior::apply(World& world, Entity entity, Transform& transform, float delta)
	{
		if ((!enabled)) // || (target == null)
		{
			return;
		}

		//auto up = world.get_up_vector();

		auto& registry = world.get_registry();

		auto target_transform = world.get_transform(target);

		switch (mode)
		{
			case Mode::LookAt:
				look_at(transform, target_transform, delta);

				break;
			case Mode::LookAt_Immediate:
				look_at_immediate(transform, target_transform);

				break;
			case Mode::Yaw:
				look_at_yaw(transform, target_transform, delta);

				break;
			case Mode::Yaw_Immediate:
				look_at_yaw_immediate(transform, target_transform);

				break;
		}

		if (!allow_roll)
		{
			transform.set_rz(0.0f);
		}
	}
}