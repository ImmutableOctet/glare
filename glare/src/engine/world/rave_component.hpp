#pragma once

#include "entity.hpp"

namespace engine
{
	class World;
	struct ModelComponent;

	struct RaveComponent
	{
		static void update(World& world);

		bool enabled = true;

		void apply(World& world, Entity entity, ModelComponent& model);
	};
}