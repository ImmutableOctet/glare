#pragma once

#include "target_behavior.hpp"

#include <engine/types.hpp>

namespace engine
{
	class World;
	struct Transform;

	struct BillboardBehavior : public TargetBehavior
	{
		using Mode = TargetBehavior::Mode;
		using TargetBehavior::resolve_mode;

		static void on_update(World& world, float delta);

		void apply(World& world, Entity entity, Transform& transform, float delta, bool update_target=true);
	};
}