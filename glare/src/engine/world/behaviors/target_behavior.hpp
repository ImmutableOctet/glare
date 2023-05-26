#pragma once

#include <engine/types.hpp>
#include <math/types.hpp>

#include <string>

namespace engine
{
	class World;
	struct Transform;

	// TODO: Rename.
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

		Transform& look_at(Transform& transform, Transform& target_transform, float delta);
		Transform& look_at_immediate(Transform& transform, Transform& target_transform);

		float look_at_yaw(Transform& transform, Transform& target_transform, float delta);
		float look_at_yaw_immediate(Transform& transform, Transform& target_transform);

		void apply(World& world, Entity entity, Transform& transform, float delta);

		Entity target = null;
		
		float interpolation = 0.5f;

		Mode mode = Mode::Default;

		bool allow_roll : 1 = true;

		// TODO: Deprecate.
		bool enabled    : 1 = true;

		inline bool get_allow_roll() const { return allow_roll; }
		inline void set_allow_roll(bool value) { allow_roll = value; }

		inline bool get_enabled() const { return enabled; }
		inline void set_enabled(bool value) { enabled = value; }
	};
}