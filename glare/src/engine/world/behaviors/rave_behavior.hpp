#pragma once

#include <engine/types.hpp>

namespace engine
{
	class World;
	struct ModelComponent;

	// Cyclically changes the color of an entity's attached model.
	struct RaveBehavior
	{
		static void on_update(World& world, float delta);

		bool enabled = true;

		void apply(World& world, Entity entity, ModelComponent& model);
	};
}