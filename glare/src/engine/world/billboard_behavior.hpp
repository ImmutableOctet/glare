#pragma once

#include "entity.hpp"

namespace engine
{
	class World;

	struct BillboardBehavior
	{
		static void update(World& world);

		void apply(World& world, Entity entity, Transform& transform);

		bool enabled = true;
	};
}