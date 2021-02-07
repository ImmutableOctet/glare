#pragma once

#include "entity.hpp"
#include "target_component.hpp"

namespace engine
{
	class World;

	struct BillboardBehavior : public TargetComponent
	{
		using Mode = TargetComponent::Mode;

		using TargetComponent::resolve_mode;

		static void update(World& world);

		void apply(World& world, Entity entity, Transform& transform, bool update_target=true);
	};
}