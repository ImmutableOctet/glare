#pragma once

#include "entity.hpp"

namespace engine
{
	class World;

	struct TargetComponent
	{
		static void update(World& world);

		Entity target;
		float interpolation = 0.5f;

		void apply(World& world, Entity entity, Transform& transform);
	};
}