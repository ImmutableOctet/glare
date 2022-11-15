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

		// TODO: Remove this flag. (Disabled behaviors should be removed from entities)
		bool enabled : 1 = true;

		//float speed = 1.0f;

		inline bool get_enabled() const { return enabled; }
		inline void set_enabled(bool value) { enabled = value; }

		void apply(World& world, Entity entity, ModelComponent& model);
	};
}