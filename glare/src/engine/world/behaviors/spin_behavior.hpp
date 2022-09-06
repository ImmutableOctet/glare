#pragma once

#include <types.hpp>

#include <math/math.hpp>
#include <engine/types.hpp>

namespace engine
{
	struct Transform;

	// Continually rotates an entity according to its configured `spin_vector`.
	struct SpinBehavior
	{
		static void on_update(World& world, float delta);

		math::Vector spin_vector = {0.0f, 0.05f, 0.0f};

		void apply(World& world, Entity entity, Transform& transform, float delta);
	};
}