#pragma once

#include <engine/types.hpp>

namespace engine
{
	class World;
	struct Transform;

	struct TargetBehavior
	{
		enum class Mode : std::uint8_t
		{
			LookAt,
			LookAt_Immediate,

			Yaw,
			Yaw_Immediate,

			Default=LookAt_Immediate
		};
		
		static Mode resolve_mode(const std::string& mode_name);

		static void on_update(World& world, float delta);

		math::Quaternion look_at(Transform& transform, Transform& target_transform, float delta);
		math::RotationMatrix look_at_immediate(Transform& transform, Transform& target_transform);

		float look_at_yaw(Transform& transform, Transform& target_transform, float delta);
		float look_at_yaw_immediate(Transform& transform, Transform& target_transform);

		void apply(World& world, Entity entity, Transform& transform, float delta);

		Entity target;
		
		float interpolation = 0.5f;

		Mode mode = Mode::Default;

		bool allow_roll = true;
		bool enabled    = true;
	};
}